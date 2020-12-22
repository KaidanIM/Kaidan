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

#include "Kaidan.h"

// Qt
#include <QGuiApplication>
#include <QThread>
#include <QTimer>
// QXmpp
#include "qxmpp-exts/QXmppUri.h"
// Kaidan
#include "AccountManager.h"
#include "AvatarFileStorage.h"
#include "CredentialsValidator.h"
#include "Database.h"
#include "Globals.h"
#include "MessageDb.h"
#include "Notifications.h"
#include "RosterDb.h"

Kaidan *Kaidan::s_instance;

Kaidan::Kaidan(QGuiApplication *app, bool enableLogging, QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(!s_instance);
	s_instance = this;

	initializeDatabase();
	initializeCaches();
	initializeClientWorker(enableLogging);

	connect(app, &QGuiApplication::applicationStateChanged, this, &Kaidan::handleApplicationStateChanged);

	// Log out of the server when the application window is closed.
	connect(app, &QGuiApplication::lastWindowClosed, this, [this]() {
		emit logOutRequested(true);
	});
}

Kaidan::~Kaidan()
{
	delete m_caches;
	delete m_database;
	s_instance = nullptr;
}

void Kaidan::logIn()
{
	emit logInRequested();
}

void Kaidan::requestRegistrationForm()
{
	emit registrationFormRequested();
}

void Kaidan::logOut()
{
	emit logOutRequested();
}

void Kaidan::handleApplicationStateChanged(Qt::ApplicationState applicationState)
{
	if (applicationState == Qt::ApplicationActive)
		emit applicationWindowActiveChanged(true);
	else
		emit applicationWindowActiveChanged(false);
}

void Kaidan::setConnectionState(Enums::ConnectionState connectionState)
{
	if (m_connectionState != connectionState) {
		m_connectionState = connectionState;
		emit connectionStateChanged();

		// Open the possibly cached URI when connected.
		// This is needed because the XMPP URIs can't be opened when Kaidan is not connected.
		if (m_connectionState == ConnectionState::StateConnected && !m_openUriCache.isEmpty()) {
			// delay is needed because sometimes the RosterPage needs to be loaded first
			QTimer::singleShot(300, this, [=] () {
				emit xmppUriReceived(m_openUriCache);
				m_openUriCache = "";
			});
		}
	}
}

void Kaidan::setConnectionError(ClientWorker::ConnectionError error)
{
	m_connectionError = error;
	emit connectionErrorChanged(error);
}

bool Kaidan::notificationsMuted(const QString &jid)
{
	return m_caches->settings->value(QString(KAIDAN_SETTINGS_NOTIFICATIONS_MUTED) + jid, false).toBool();
}

void Kaidan::setNotificationsMuted(const QString &jid, bool muted)
{
	m_caches->settings->setValue(QString(KAIDAN_SETTINGS_NOTIFICATIONS_MUTED) + jid, muted);
	emit notificationsMutedChanged(jid);
}

void Kaidan::setJid(const QString &jid)
{
	m_client->accountManager()->setJid(jid);
}

QString Kaidan::jid() const
{
	return m_client->accountManager()->jid();
}

void Kaidan::setPassword(const QString &password)
{
	m_client->accountManager()->setPassword(password);
}

QString Kaidan::password() const
{
	return m_client->accountManager()->password();
}

void Kaidan::setHost(const QString &host)
{
	m_client->accountManager()->setHost(host);
}

void Kaidan::resetHost()
{
	m_client->accountManager()->resetHost();
}

QString Kaidan::host() const
{
	return m_client->accountManager()->host();
}

void Kaidan::setPort(int port)
{
	m_client->accountManager()->setPort(port);
}

void Kaidan::resetPort()
{
	m_client->accountManager()->resetPort();
}

int Kaidan::port() const
{
	return m_client->accountManager()->port();
}

void Kaidan::setCustomConnectionSettingsEnabled(bool enabled)
{
	m_client->accountManager()->setCustomConnectionSettingsEnabled(enabled);
}

bool Kaidan::customConnectionSettingsEnabled() const
{
	return m_client->accountManager()->customConnectionSettingsEnabled();
}

void Kaidan::setPasswordVisibility(PasswordVisibility passwordVisibility)
{
	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY, quint8(passwordVisibility));
	emit passwordVisibilityChanged();
}

Kaidan::PasswordVisibility Kaidan::passwordVisibility() const
{
	return m_caches->settings
		->value(KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY, PasswordVisible)
		.value<Kaidan::PasswordVisibility>();
}

quint8 Kaidan::connectionError() const
{
	return quint8(m_connectionError);
}

void Kaidan::addOpenUri(const QString &uri)
{
	if (!QXmppUri::isXmppUri(uri))
		return;

	if (m_connectionState == ConnectionState::StateConnected) {
		emit xmppUriReceived(uri);
	} else {
		//: The link is an XMPP-URI (i.e. 'xmpp:kaidan@muc.kaidan.im?join' for joining a chat)
		emit passiveNotificationRequested(tr("The link will be opened after you have connected."));
		m_openUriCache = uri;
	}
}

quint8 Kaidan::logInByUri(const QString &uri)
{
	if (!QXmppUri::isXmppUri(uri)) {
		notifyForInvalidLoginUri();
		return quint8(LoginByUriState::InvalidLoginUri);
	}

	QXmppUri parsedUri(uri);

	if (!CredentialsValidator::isAccountJidValid(parsedUri.jid())) {
		notifyForInvalidLoginUri();
		return quint8(LoginByUriState::InvalidLoginUri);
	}

	setJid(parsedUri.jid());

	if (parsedUri.action() != QXmppUri::Login || !CredentialsValidator::isPasswordValid(parsedUri.password())) {
		return quint8(LoginByUriState::PasswordNeeded);
	}

	// Connect with the extracted credentials.
	setPassword(parsedUri.password());
	logIn();
	return quint8(LoginByUriState::Connecting);
}

void Kaidan::initializeDatabase()
{
	m_dbThrd = new QThread();
	m_dbThrd->setObjectName("SqlDatabase");

	m_database = new Database();
	m_database->moveToThread(m_dbThrd);

	m_msgDb = new MessageDb();
	m_msgDb->moveToThread(m_dbThrd);

	m_rosterDb = new RosterDb(m_database);
	m_rosterDb->moveToThread(m_dbThrd);

	connect(m_dbThrd, &QThread::started, m_database, &Database::openDatabase);
	m_dbThrd->start();
}

void Kaidan::initializeCaches()
{
	m_caches = new ClientWorker::Caches(m_rosterDb, m_msgDb, this);

	// Connect the avatar changed signal of the avatarStorage with the NOTIFY signal
	// of the Q_PROPERTY for the avatar storage (so all avatars are updated in QML)
	connect(m_caches->avatarStorage, &AvatarFileStorage::avatarIdsChanged, this, &Kaidan::avatarStorageChanged);
}

void Kaidan::initializeClientWorker(bool enableLogging)
{
	m_cltThrd = new QThread();
	m_cltThrd->setObjectName("XmppClient");

	m_client = new ClientWorker(m_caches, enableLogging);
	m_client->moveToThread(m_cltThrd);

	connect(m_client->accountManager(), &AccountManager::jidChanged, this, &Kaidan::jidChanged);
	connect(m_client->accountManager(), &AccountManager::passwordChanged, this, &Kaidan::passwordChanged);
	connect(m_client->accountManager(), &AccountManager::newCredentialsNeeded, this, &Kaidan::newCredentialsNeeded);

	connect(m_client, &ClientWorker::loggedInWithNewCredentials, this, &Kaidan::loggedInWithNewCredentials);
	connect(m_client, &ClientWorker::connectionStateChanged, this, &Kaidan::setConnectionState);
	connect(m_client, &ClientWorker::connectionErrorChanged, this, &Kaidan::setConnectionError);
	connect(m_client, &ClientWorker::showMessageNotificationRequested, this, [](const QString &senderJid, const QString &senderName, const QString &message) {
		Notifications::sendMessageNotification(senderJid, senderName, message);
	});

	connect(m_cltThrd, &QThread::started, m_client, &ClientWorker::initialize);
	m_cltThrd->start();
}

void Kaidan::notifyForInvalidLoginUri()
{
	qWarning() << "[main]" << "No valid login URI found.";
	emit passiveNotificationRequested(tr("No valid login QR code found."));
}

ClientWorker *Kaidan::client() const
{
	return m_client;
}

RosterDb *Kaidan::rosterDb() const
{
	return m_rosterDb;
}

MessageDb *Kaidan::messageDb() const
{
	return m_msgDb;
}

Kaidan *Kaidan::instance()
{
	return s_instance;
}
