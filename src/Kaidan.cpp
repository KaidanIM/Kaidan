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

#include "Kaidan.h"

// Qt
#include <QGuiApplication>
#include <QSettings>
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

Kaidan::Kaidan(bool enableLogging, QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(!s_instance);
	s_instance = this;

	initializeDatabase();
	initializeCaches();
	initializeClientWorker(enableLogging);

	// Log out of the server when the application window is closed.
	connect(qGuiApp, &QGuiApplication::lastWindowClosed, this, [this]() {
		emit logOutRequested(true);
	});
}

Kaidan::~Kaidan()
{
	delete m_caches;
	delete m_database;
	s_instance = nullptr;
}

QSize Kaidan::applicationWindowSize() const
{
	return m_caches->settings->value(KAIDAN_SETTINGS_WINDOW_SIZE).toSize();
}

void Kaidan::storeApplicationWindowSize(const QSize &size) const
{
	m_caches->settings->setValue(KAIDAN_SETTINGS_WINDOW_SIZE, size);
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
	if (error != m_connectionError) {
		m_connectionError = error;
		emit connectionErrorChanged();
	}
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
		return quint8(LoginByUriState::InvalidLoginUri);
	}

	QXmppUri parsedUri(uri);

	if (!CredentialsValidator::isAccountJidValid(parsedUri.jid())) {
		return quint8(LoginByUriState::InvalidLoginUri);
	}

	AccountManager::instance()->setJid(parsedUri.jid());

	if (parsedUri.action() != QXmppUri::Login || !CredentialsValidator::isPasswordValid(parsedUri.password())) {
		return quint8(LoginByUriState::PasswordNeeded);
	}

	// Connect with the extracted credentials.
	AccountManager::instance()->setPassword(parsedUri.password());
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
	m_caches = new ClientWorker::Caches(this);

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

	connect(AccountManager::instance(), &AccountManager::credentialsNeeded, this, &Kaidan::credentialsNeeded);

	connect(m_client, &ClientWorker::loggedInWithNewCredentials, this, &Kaidan::openChatViewRequested);
	connect(m_client, &ClientWorker::connectionStateChanged, this, &Kaidan::setConnectionState);
	connect(m_client, &ClientWorker::connectionErrorChanged, this, &Kaidan::setConnectionError);

	m_cltThrd->start();
}

Kaidan *Kaidan::instance()
{
	return s_instance;
}

Database *Kaidan::database() const
{
	return m_database;
}
