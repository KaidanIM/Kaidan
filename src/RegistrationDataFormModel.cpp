/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

#include "RegistrationDataFormModel.h"
// Qt
#include <QList>
#include <QString>
// QXmpp
#include <QXmppDataForm.h>

#define USERNAME "username"
#define PASSWORD "password"
#define EMAIL "email"

// fields to be filtered out
#define FORM_TYPE "FORM_TYPE"
#define FROM "from"
#define CAPTCHA_FALLBACK_TEXT "captcha-fallback-text"
#define CAPTCHA_FALLBACK_URL "captcha-fallback-url"
#define CAPTCHAHIDDEN "captchahidden"
#define CHALLENGE "challenge"
#define SID "sid"

RegistrationDataFormModel::RegistrationDataFormModel(QObject *parent)
    : DataFormModel(parent)
{
	initializeFilteredDataFormFields();
}

RegistrationDataFormModel::RegistrationDataFormModel(const QXmppDataForm &dataForm, QObject *parent)
    : DataFormModel(dataForm, parent)
{
	initializeFilteredDataFormFields();
}

bool RegistrationDataFormModel::hasUsernameField() const
{
	return usernameFieldIndex() != -1;
}

bool RegistrationDataFormModel::hasPasswordField() const
{
	return passwordFieldIndex() != -1;
}

bool RegistrationDataFormModel::hasEmailField() const
{
	return emailFieldIndex() != -1;
}

void RegistrationDataFormModel::setUsername(const QString &username)
{
	setData(index(usernameFieldIndex()), username, DataFormModel::Value);
}

void RegistrationDataFormModel::setPassword(const QString &password)
{
	setData(index(passwordFieldIndex()), password, DataFormModel::Value);
}

void RegistrationDataFormModel::setEmail(const QString& email)
{
	setData(index(emailFieldIndex()), email, DataFormModel::Value);
}

int RegistrationDataFormModel::usernameFieldIndex() const
{
	const QList<QXmppDataForm::Field> &fields = m_form.fields();
	for (int i = 0; i < fields.size(); i++) {
		if (fields.at(i).type() == QXmppDataForm::Field::TextSingleField &&
				fields.at(i).key().compare(USERNAME, Qt::CaseInsensitive) == 0) {
			return i;
		}
	}
	return -1;
}

int RegistrationDataFormModel::passwordFieldIndex() const
{
	const QList<QXmppDataForm::Field> &fields = m_form.fields();
	for (int i = 0; i < fields.size(); i++) {
		if (fields.at(i).type() == QXmppDataForm::Field::TextPrivateField &&
				fields.at(i).key().compare(PASSWORD, Qt::CaseInsensitive) == 0) {
			return i;
		}
	}
	return -1;
}

int RegistrationDataFormModel::emailFieldIndex() const
{
	const QList<QXmppDataForm::Field> &fields = m_form.fields();
	for (int i = 0; i < fields.size(); i++) {
		if (fields.at(i).type() == QXmppDataForm::Field::TextSingleField &&
				fields.at(i).key().compare(EMAIL, Qt::CaseInsensitive) == 0) {
			return i;
		}
	}
	return -1;
}

QXmppDataForm::Field RegistrationDataFormModel::extractUsernameField() const
{
	int index = usernameFieldIndex();
	return index >= 0 ? m_form.fields().at(index) : QXmppDataForm::Field();
}

QXmppDataForm::Field RegistrationDataFormModel::extractPasswordField() const
{
	int index = passwordFieldIndex();
	return index >= 0 ? m_form.fields().at(index) : QXmppDataForm::Field();
}

QXmppDataForm::Field RegistrationDataFormModel::extractEmailField() const
{
	int index = emailFieldIndex();
	return index >= 0 ? m_form.fields().at(index) : QXmppDataForm::Field();
}

QString RegistrationDataFormModel::extractUsername() const
{
	return extractUsernameField().value().toString();
}

QString RegistrationDataFormModel::extractPassword() const
{
	return extractPasswordField().value().toString();
}

QString RegistrationDataFormModel::extractEmail() const
{
	return extractEmailField().value().toString();
}

bool RegistrationDataFormModel::isFakeForm() const
{
	return m_isFakeForm;
}

void RegistrationDataFormModel::setIsFakeForm(bool isFakeForm)
{
	m_isFakeForm = isFakeForm;
}

QVector<int> RegistrationDataFormModel::indiciesToFilter() const
{
	QVector<int> indicies;

	// username and password
	// email is currently not filtered because we do not have an extra email view
	for (const auto &index : {usernameFieldIndex(), passwordFieldIndex()}) {
		if (index != -1)
			indicies << index;
	}

	// search for other common fields to filter for
	for (int i = 0; i < m_form.fields().size(); i++) {
		QString key = m_form.fields().at(i).key();
		if (m_filteredDataFormFields.contains(key))
			indicies << i;
	}

	return indicies;
}

void RegistrationDataFormModel::initializeFilteredDataFormFields()
{
	m_filteredDataFormFields = {
		FORM_TYPE,
		FROM,
		CAPTCHA_FALLBACK_TEXT,
		CAPTCHA_FALLBACK_URL,
		CAPTCHAHIDDEN,
		CHALLENGE,
		SID
	};
}
