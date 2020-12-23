/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RegistrationManager.h"

// Qt
#include <QDomElement>
#include <QList>
#include <QObject>
#include <QSettings>
// QXmpp
#include <QXmppBitsOfBinaryDataList.h>
#include <QXmppClient.h>
#include <QXmppRegisterIq.h>
#include <QXmppRegistrationManager.h>
// Kaidan
#include "AccountManager.h"
#include "BitsOfBinaryImageProvider.h"
#include "ClientWorker.h"
#include "Globals.h"
#include "Kaidan.h"
#include "RegistrationDataFormModel.h"

RegistrationManager::RegistrationManager(ClientWorker *clientWorker, QXmppClient *client, QSettings *settings, QObject *parent)
	: QObject(parent),
	  m_clientWorker(clientWorker),
	  m_client(client),
	  m_settings(settings),
	  m_manager(new QXmppRegistrationManager),
	  m_dataFormModel(new RegistrationDataFormModel())
{
	client->addExtension(m_manager);

	connect(m_manager, &QXmppRegistrationManager::supportedByServerChanged, this, &RegistrationManager::handleInBandRegistrationSupportedChanged);

	// account creation
	connect(Kaidan::instance(), &Kaidan::sendRegistrationForm, this, &RegistrationManager::sendRegistrationForm);
	connect(m_manager, &QXmppRegistrationManager::registrationFormReceived, this, &RegistrationManager::handleRegistrationFormReceived);
	connect(m_manager, &QXmppRegistrationManager::registrationSucceeded, this, &RegistrationManager::handleRegistrationSucceeded);
	connect(m_manager, &QXmppRegistrationManager::registrationFailed, this, &RegistrationManager::handleRegistrationFailed);

	// account deletion
	connect(m_manager, &QXmppRegistrationManager::accountDeletionFailed, m_clientWorker, &ClientWorker::handleAccountDeletionFromServerFailed);
	connect(m_manager, &QXmppRegistrationManager::accountDeleted, m_clientWorker, &ClientWorker::handleAccountDeletedFromServer);

	// password change
	connect(Kaidan::instance(), &Kaidan::changePassword, this, &RegistrationManager::changePassword);
	connect(m_manager, &QXmppRegistrationManager::passwordChanged, this, &RegistrationManager::handlePasswordChanged);
	connect(m_manager, &QXmppRegistrationManager::passwordChangeFailed, this, &RegistrationManager::handlePasswordChangeFailed);
}

void RegistrationManager::setRegisterOnConnectEnabled(bool registerOnConnect)
{
	m_manager->setRegisterOnConnectEnabled(registerOnConnect);
}

void RegistrationManager::deleteAccount()
{
	m_manager->deleteAccount();
}

void RegistrationManager::sendRegistrationForm()
{
	if (m_dataFormModel->isFakeForm()) {
		QXmppRegisterIq iq;
		iq.setUsername(m_dataFormModel->extractUsername());
		iq.setPassword(m_dataFormModel->extractPassword());
		iq.setEmail(m_dataFormModel->extractEmail());

		m_manager->setRegistrationFormToSend(iq);
	} else {
		m_manager->setRegistrationFormToSend(m_dataFormModel->form());
	}

	m_clientWorker->connectToRegister();
}

void RegistrationManager::changePassword(const QString &newPassword)
{
	m_clientWorker->startTask(
		[=] () {
			m_manager->changePassword(newPassword);
		}
	);
}

void RegistrationManager::handleInBandRegistrationSupportedChanged()
{
	if (m_client->isConnected()) {
		m_clientWorker->caches()->serverFeaturesCache->setInBandRegistrationSupported(m_manager->supportedByServer());
	}
}

void RegistrationManager::handleRegistrationFormReceived(const QXmppRegisterIq &iq)
{
	m_client->disconnectFromServer();

	bool isFakeForm;
	QXmppDataForm newDataForm = extractFormFromRegisterIq(iq, isFakeForm);

	// If the data form is not set, there is a problem with the server.
	if (newDataForm.fields().isEmpty()) {
		emit m_clientWorker->connectionErrorChanged(ClientWorker::RegistrationUnsupported);
		return;
	}

	copyUserDefinedValuesToNewForm(m_dataFormModel->form(), newDataForm);
	cleanUpLastForm();

	m_dataFormModel = new RegistrationDataFormModel(newDataForm);
	m_dataFormModel->setIsFakeForm(isFakeForm);
	// Move to the main thread, so QML can connect signals to the model.
	m_dataFormModel->moveToThread(Kaidan::instance()->thread());

	// Add the attached Bits of Binary data to the corresponding image provider.
	const auto bobDataList = iq.bitsOfBinaryData();
	for (const auto &bobData : bobDataList) {
		BitsOfBinaryImageProvider::instance()->addImage(bobData);
		m_contentIdsToRemove << bobData.cid();
	}

	emit Kaidan::instance()->registrationFormReceived(m_dataFormModel);
}

void RegistrationManager::handleRegistrationSucceeded()
{
	m_clientWorker->accountManager()->setJid(m_dataFormModel->extractUsername().append('@').append(m_client->configuration().domain()));
	m_clientWorker->accountManager()->setPassword(m_dataFormModel->extractPassword());

	m_client->disconnectFromServer();
	setRegisterOnConnectEnabled(false);
	m_clientWorker->logIn();

	cleanUpLastForm();
	m_dataFormModel = new RegistrationDataFormModel();
}

void RegistrationManager::handleRegistrationFailed(const QXmppStanza::Error &error)
{
	m_client->disconnectFromServer();
	setRegisterOnConnectEnabled(false);

	RegistrationError registrationError = RegistrationError::UnknownError;

	switch(error.type()) {
	case QXmppStanza::Error::Cancel:
		if (error.condition() == QXmppStanza::Error::FeatureNotImplemented)
			registrationError = RegistrationError::InBandRegistrationNotSupported;
		else if (error.condition() == QXmppStanza::Error::Conflict)
			registrationError = RegistrationError::UsernameConflict;
		else if (error.condition() == QXmppStanza::Error::NotAllowed && error.text().contains("captcha", Qt::CaseInsensitive))
			registrationError = RegistrationError::CaptchaVerificationFailed;
		break;
	case QXmppStanza::Error::Modify:
		if (error.condition() == QXmppStanza::Error::NotAcceptable) {
			// TODO: Check error text in English (needs QXmpp change)
			if (error.text().contains("password", Qt::CaseInsensitive) && (error.text().contains("weak", Qt::CaseInsensitive) || error.text().contains("short", Qt::CaseInsensitive)))
				registrationError = RegistrationError::PasswordTooWeak;
			else if (error.text().contains("ip", Qt::CaseInsensitive) || error.text().contains("quickly", Qt::CaseInsensitive)
)
				registrationError = RegistrationError::TemporarilyBlocked;
			else
				registrationError = RegistrationError::RequiredInformationMissing;
		} else if (error.condition() == QXmppStanza::Error::BadRequest && error.text().contains("captcha", Qt::CaseInsensitive)) {
			registrationError = RegistrationError::CaptchaVerificationFailed;
		}
		break;
	default:
#if QXMPP_VERSION == QT_VERSION_CHECK(1, 2, 0)
		// Workaround: Catch an error which is wrongly emitted by QXmpp
		// v1.2.0 although the registration was successful.
		if (error.text().isEmpty())
			return;
#endif
		break;
	}

	emit Kaidan::instance()->registrationFailed(quint8(registrationError), error.text());
}

void RegistrationManager::handlePasswordChanged(const QString &newPassword)
{
	m_clientWorker->accountManager()->setPassword(newPassword);
	m_clientWorker->accountManager()->storePassword();
	m_clientWorker->accountManager()->setHasNewCredentials(false);
	m_clientWorker->finishTask();
}

void RegistrationManager::handlePasswordChangeFailed(const QXmppStanza::Error &error)
{
	emit Kaidan::instance()->passwordChangeFailed(error.text());
	m_clientWorker->finishTask();
}

QXmppDataForm RegistrationManager::extractFormFromRegisterIq(const QXmppRegisterIq& iq, bool &isFakeForm)
{
	QXmppDataForm newDataForm = iq.form();
	if (newDataForm.fields().isEmpty()) {
		// This is a hack, so we only need to implement one way of registering in QML.
		// A 'fake' data form model is created with a username and password field.
		isFakeForm = true;

		if (!iq.username().isNull()) {
			QXmppDataForm::Field field;
			field.setKey(QStringLiteral("username"));
			field.setRequired(true);
			field.setType(QXmppDataForm::Field::TextSingleField);
			newDataForm.fields().append(field);
		}

		if (!iq.password().isNull()) {
			QXmppDataForm::Field field;
			field.setKey(QStringLiteral("password"));
			field.setRequired(true);
			field.setType(QXmppDataForm::Field::TextPrivateField);
			newDataForm.fields().append(field);
		}

		if (!iq.email().isNull()) {
			QXmppDataForm::Field field;
			field.setKey(QStringLiteral("email"));
			field.setRequired(true);
			field.setType(QXmppDataForm::Field::TextPrivateField);
			newDataForm.fields().append(field);
		}
	} else {
		isFakeForm = false;
	}

	return newDataForm;
}

void RegistrationManager::copyUserDefinedValuesToNewForm(const QXmppDataForm &oldForm, QXmppDataForm& newForm)
{
	// Copy values from the last form.
	const QList<QXmppDataForm::Field> oldFields = oldForm.fields();
	for (const auto &field : oldFields) {
		// Only copy fields which:
		//  * are required,
		//  * are visible to the user
		//  * do not have a media element (e.g. CAPTCHA)
		if (field.isRequired() && field.type() != QXmppDataForm::Field::HiddenField &&
			field.mediaSources().isEmpty()) {
			for (auto &fieldFromNewForm : newForm.fields()) {
				if (fieldFromNewForm.key() == field.key()) {
					fieldFromNewForm.setValue(field.value());
					break;
				}
			}
		}
	}
}

void RegistrationManager::cleanUpLastForm()
{
	delete m_dataFormModel;

	// Remove content IDs from the last form.
	for (const auto &cid : qAsConst(m_contentIdsToRemove))
		BitsOfBinaryImageProvider::instance()->removeImage(cid);
}
