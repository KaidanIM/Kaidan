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

#include "ClientWorker.h"
// Qt
#include <QDebug>
#include <QGuiApplication>
#include <QSettings>
#include <QSysInfo>
// QXmpp
#include <QXmppClient.h>
#include <QXmppConfiguration.h>
#include <QXmppPresence.h>
#include <QXmppVersionManager.h>
// Kaidan
#include "DiscoveryManager.h"
#include "DownloadManager.h"
#include "Kaidan.h"
#include "LogHandler.h"
#include "MessageDb.h"
#include "MessageHandler.h"
#include "RegistrationManager.h"
#include "RosterDb.h"
#include "RosterManager.h"
#include "UploadManager.h"
#include "VCardManager.h"

ClientWorker::ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QGuiApplication *app,
                           QObject* parent)
        : QObject(parent), caches(caches), kaidan(kaidan), enableLogging(enableLogging), app(app)
{
	client = new QXmppClient(this);
	logger = new LogHandler(client, this);
	logger->enableLogging(enableLogging);
	vCardManager = new VCardManager(client, caches->avatarStorage, this);
	registrationManager = new RegistrationManager(kaidan, this, client, caches->settings);
	rosterManager = new RosterManager(kaidan, client,  caches->rosterModel,
	                                  caches->avatarStorage, vCardManager, this);
	msgHandler = new MessageHandler(kaidan, client, caches->msgModel, this);
	discoManager = new DiscoveryManager(client, this);
	uploadManager = new UploadManager(client, rosterManager, this);
	downloadManager = new DownloadManager(kaidan, caches->transferCache,
	                                      caches->msgModel, this);

	client->addExtension(registrationManager);

	connect(client, &QXmppClient::presenceReceived,
	        caches->presCache, &PresenceCache::updatePresence);
	connect(client, &QXmppClient::disconnected,
	        caches->presCache, &PresenceCache::clear);

	connect(this, &ClientWorker::credentialsUpdated, this, &ClientWorker::setCredentials);

	// publish kaidan version
	client->versionManager().setClientName(APPLICATION_DISPLAY_NAME);
	client->versionManager().setClientVersion(VERSION_STRING);
	client->versionManager().setClientOs(QSysInfo::prettyProductName());

	// Client State Indication
	connect(app, &QGuiApplication::applicationStateChanged, this, &ClientWorker::setCsiState);

	// account deletion
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->rosterDb(), &RosterDb::clearAll);
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->messageDb(), &MessageDb::removeAllMessages);
}

VCardManager *ClientWorker::getVCardManager() const
{
	return vCardManager;
}

void ClientWorker::main()
{
	// initialize random generator
	qsrand(time(nullptr));

	connect(client, &QXmppClient::stateChanged, kaidan, &Kaidan::setConnectionState);

	connect(client, &QXmppClient::connected, this, &ClientWorker::onConnected);
	connect(client, &QXmppClient::disconnected, this, &ClientWorker::onDisconnected);

	connect(client, &QXmppClient::error, this, &ClientWorker::onConnectionError);

	connect(this, &ClientWorker::connectRequested, this, &ClientWorker::xmppConnect);
	connect(this, &ClientWorker::disconnectRequested, client, &QXmppClient::disconnectFromServer);
}

void ClientWorker::xmppConnect()
{
	QXmppConfiguration config;
	config.setJid(creds.jid);
	config.setResource(creds.jidResource.append(".").append(generateRandomString()));
	config.setPassword(creds.password);
	config.setAutoAcceptSubscriptions(false);
	config.setStreamSecurityMode(QXmppConfiguration::TLSRequired);
	config.setAutoReconnectionEnabled(true); // will automatically reconnect
	// on first try we must be sure that we connect successfully
	// otherwise this could end in a reconnection loop
	if (creds.isFirstTry)
		config.setAutoReconnectionEnabled(false);

	client->connectToServer(config, QXmppPresence(QXmppPresence::Available));
}

void ClientWorker::deleteAccountFromClientAndServer()
{
	m_isAccountToBeDeletedFromClientAndServer = true;

	// If the client is already connected, delete the account directly from the server.
	// Otherwise, connect first and delete the account afterwards.
	if (client->isAuthenticated()) {
		registrationManager->deleteAccount();
	} else {
		m_isClientConnectedBeforeAccountDeletionFromServer = false;
		xmppConnect();
	}
}

void ClientWorker::deleteAccountFromClient()
{
	// If the client is already disconnected, delete the account directly from the client.
	// Otherwise, disconnect first and delete the account afterwards.
	if (!client->isAuthenticated()) {
		emit deleteAccountFromDatabase();
		kaidan->deleteCredentials();
		m_isAccountToBeDeletedFromClient = false;
	} else {
		m_isAccountToBeDeletedFromClient = true;
		emit disconnectRequested();
	}
}

void ClientWorker::onAccountDeletedFromServer()
{
	m_isAccountDeletedFromServer = true;
}

void ClientWorker::onAccountDeletionFromServerFailed(QXmppStanza::Error error)
{
	emit kaidan->passiveNotificationRequested(tr("Your account could not be deleted from the server. Therefore, it was also not removed from this app: %1").arg(error.text()));

	m_isAccountToBeDeletedFromClientAndServer = false;

	if (!m_isClientConnectedBeforeAccountDeletionFromServer)
		emit disconnectRequested();
}

void ClientWorker::onConnected()
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Connected successfully to server";

	// Emit signal, that logging in with these credentials has worked for the first time
	if (creds.isFirstTry)
		emit kaidan->loggedInWithNewCredentials();

	// accept credentials and save them
	creds.isFirstTry = false;
	caches->settings->setValue(KAIDAN_SETTINGS_AUTH_JID, creds.jid);
	caches->settings->setValue(KAIDAN_SETTINGS_AUTH_PASSWD,
	                           QString::fromUtf8(creds.password.toUtf8().toBase64()));

	// after first log in we always want to automatically reconnect
	client->configuration().setAutoReconnectionEnabled(true);

	// If the account could not be deleted from the server because the client was disconnected, delete it now.
	if (m_isAccountToBeDeletedFromClientAndServer)
		registrationManager->deleteAccount();
}

void ClientWorker::onDisconnected()
{
	// Delete the account from the client if the client was connected and had to disconnect first or if the account was deleted from the server.
	if (m_isAccountToBeDeletedFromClient || (m_isAccountToBeDeletedFromClientAndServer && m_isAccountDeletedFromServer)) {
		m_isAccountToBeDeletedFromClientAndServer = false;
		m_isAccountDeletedFromServer = false;

		deleteAccountFromClient();
	}
}

void ClientWorker::onConnectionError(QXmppClient::Error error)
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Disconnected:" << error;

	// Check if first time connecting with these credentials
	if (creds.isFirstTry || error == QXmppClient::XmppStreamError) {
		// always request new credentials, when failed to connect on first time
		emit kaidan->newCredentialsNeeded();
	}

	QXmppStanza::Error::Condition xmppStreamError;
	QAbstractSocket::SocketError socketError;

	switch (error) {
	case QXmppClient::NoError:
		emit connectionErrorChanged(ClientWorker::NoError);
		break;
	case QXmppClient::KeepAliveError:
		emit connectionErrorChanged(ClientWorker::KeepAliveError);
		break;
	case QXmppClient::XmppStreamError:
		xmppStreamError = client->xmppStreamError();
		if (xmppStreamError == QXmppStanza::Error::NotAuthorized) {
			emit connectionErrorChanged(ClientWorker::AuthenticationFailed);
		} else {
			emit connectionErrorChanged(ClientWorker::NotConnected);
		}
		break;
	case QXmppClient::SocketError:
		socketError = client->socketError();
		switch (socketError) {
		case QAbstractSocket::ConnectionRefusedError:
		case QAbstractSocket::RemoteHostClosedError:
			emit connectionErrorChanged(ClientWorker::ConnectionRefused);
			break;
		case QAbstractSocket::HostNotFoundError:
			emit connectionErrorChanged(ClientWorker::DnsError);
			break;
		case QAbstractSocket::SocketAccessError:
			emit connectionErrorChanged(ClientWorker::NoNetworkPermission);
			break;
		case QAbstractSocket::SocketTimeoutError:
			emit connectionErrorChanged(ClientWorker::KeepAliveError);
			break;
		case QAbstractSocket::SslHandshakeFailedError:
		case QAbstractSocket::SslInternalError:
			emit connectionErrorChanged(ClientWorker::TlsFailed);
			break;
		default:
			emit connectionErrorChanged(ClientWorker::NotConnected);
		}
		break;
	}
}

void ClientWorker::setCsiState(Qt::ApplicationState state)
{
	if (state == Qt::ApplicationActive)
		client->setActive(true);
	else
		client->setActive(false);
}

QString ClientWorker::generateRandomString(unsigned int length) const
{
	const QString resourceChars(KAIDAN_RESOURCE_RANDOM_CHARS);

	QString randomString;
	for (unsigned int i = 0; i < length; ++i)
		randomString.append(resourceChars.at(qrand() % resourceChars.length()));
	return randomString;
}
