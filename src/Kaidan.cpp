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
#include <QDebug>
#include <QGuiApplication>
#include <QSettings>
#include <QThread>
// QXmpp
#include <QXmppStanza.h>
#include "qxmpp-exts/QXmppColorGenerator.h"
#include "qxmpp-exts/QXmppUri.h"
// Kaidan
#include "AvatarFileStorage.h"
#include "CredentialsValidator.h"
#include "Database.h"
#include "MessageDb.h"
#include "MessageModel.h"
#include "Notifications.h"
#include "PresenceCache.h"
#include "QmlUtils.h"
#include "RosterDb.h"
#include "RosterModel.h"

Kaidan *Kaidan::s_instance;

Kaidan::Kaidan(QGuiApplication *app, bool enableLogging, QObject *parent)
        : QObject(parent),
          m_database(new Database()),
          m_dbThrd(new QThread()),
          m_msgDb(new MessageDb()),
          m_rosterDb(new RosterDb(m_database)),
          m_cltThrd(new QThread())
{
	Q_ASSERT(!s_instance);
	s_instance = this;

	connect(app, &QGuiApplication::applicationStateChanged, this, &Kaidan::handleApplicationStateChanged);

	// Database setup
	m_database->moveToThread(m_dbThrd);
	m_msgDb->moveToThread(m_dbThrd);
	m_rosterDb->moveToThread(m_dbThrd);

	connect(m_dbThrd, &QThread::started, m_database, &Database::openDatabase);

	m_dbThrd->setObjectName("SqlDatabase");
	m_dbThrd->start();

	// Caching components
	m_caches = new ClientWorker::Caches(this, m_rosterDb, m_msgDb, this);
	// Connect the avatar changed signal of the avatarStorage with the NOTIFY signal
	// of the Q_PROPERTY for the avatar storage (so all avatars are updated in QML)
	connect(m_caches->avatarStorage, &AvatarFileStorage::avatarIdsChanged,
	        this, &Kaidan::avatarStorageChanged);

	//
	// Load settings
	//

	setJid(m_caches->settings->value(KAIDAN_SETTINGS_AUTH_JID).toString());
	setJidResourcePrefix(m_caches->settings->value(KAIDAN_SETTINGS_AUTH_JID_RESOURCE_PREFIX).toString());
	setPassword(QByteArray::fromBase64(
		m_caches->settings->value(KAIDAN_SETTINGS_AUTH_PASSWD).toString().toUtf8()
	));
	// Use a default prefix for the JID's resource part if no prefix is already set.
	if (m_creds.jidResourcePrefix.isEmpty())
		setJidResourcePrefix(KAIDAN_JID_RESOURCE_DEFAULT_PREFIX);
	m_creds.isFirstTry = false;

	setPasswordVisibility(PasswordVisibility(
		m_caches->settings->value(KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY).toUInt()));

	//
	// Start ClientWorker on new thread
	//

	m_client = new ClientWorker(m_caches, this, enableLogging);
	m_client->setCredentials(m_creds);
	m_client->moveToThread(m_cltThrd);

	connect(m_client, &ClientWorker::connectionErrorChanged, this, &Kaidan::setConnectionError);
	connect(m_client, &ClientWorker::showMessageNotificationRequested, this, [](const QString &senderJid, const QString &senderName, const QString &message) {
		Notifications::sendMessageNotification(senderJid, senderName, message);
	});
	connect(m_cltThrd, &QThread::started, m_client, &ClientWorker::main);

	m_client->setObjectName("XmppClient");
	m_cltThrd->start();
}

Kaidan::~Kaidan()
{
	delete m_caches;
	delete m_database;
	s_instance = nullptr;
}

void Kaidan::start()
{
	if (m_creds.jid.isEmpty() || m_creds.password.isEmpty())
		emit newCredentialsNeeded();
	else
		mainConnect();
}

void Kaidan::mainConnect()
{
	emit m_client->credentialsUpdated(m_creds);
	emit m_client->connectRequested();
}

void Kaidan::requestRegistrationForm()
{
	emit m_client->credentialsUpdated(m_creds);
	emit m_client->registrationFormRequested();
}

void Kaidan::mainDisconnect()
{
	// disconnect the client if connected or connecting
	if (m_connectionState != ConnectionState::StateDisconnected)
		emit m_client->disconnectRequested();
}

void Kaidan::handleApplicationStateChanged(Qt::ApplicationState applicationState)
{
	if (applicationState == Qt::ApplicationActive)
		emit applicationWindowActiveChanged(true);
	else
		emit applicationWindowActiveChanged(false);
}

void Kaidan::setConnectionState(QXmppClient::State state)
{
	if (m_connectionState != static_cast<ConnectionState>(state)) {
		m_connectionState = static_cast<ConnectionState>(state);
		emit connectionStateChanged();

		// Open the possibly cached URI when connected.
		// This is needed because the XMPP URIs can't be opened when Kaidan is not connected.
		if (m_connectionState == ConnectionState::StateConnected && !m_openUriCache.isEmpty()) {
			// delay is needed because sometimes the RosterPage needs to be loaded first
			QTimer::singleShot(300, [=] () {
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

void Kaidan::deleteCredentials()
{
	// Delete the JID.
	m_caches->settings->remove(KAIDAN_SETTINGS_AUTH_JID);
	setJid(QString());

	// Delete the password.
	m_caches->settings->remove(KAIDAN_SETTINGS_AUTH_PASSWD);
	setPassword(QString());

	setPasswordVisibility(PasswordVisible);

	emit newCredentialsNeeded();
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
	m_creds.jid = jid;
	// credentials were modified -> first try
	m_creds.isFirstTry = true;
	emit jidChanged();
}

void Kaidan::setJidResourcePrefix(const QString &jidResourcePrefix)
{
	// JID resource won't influence the authentication, so we don't need
	// to set the first try flag and can save it.
	m_creds.jidResourcePrefix = jidResourcePrefix;

	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_JID_RESOURCE_PREFIX, jidResourcePrefix);
	emit jidResourcePrefixChanged();
}

void Kaidan::setPassword(const QString &password)
{
	m_creds.password = password;
	// credentials were modified -> first try
	m_creds.isFirstTry = true;
	emit passwordChanged();
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

	if (!parsedUri.hasAction(QXmppUri::Login) || !CredentialsValidator::isPasswordValid(parsedUri.password())) {
		return quint8(LoginByUriState::PasswordNeeded);
	}

	// Connect with the extracted credentials.
	setPassword(parsedUri.password());
	mainConnect();
	return quint8(LoginByUriState::Connecting);
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
