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
#include <QStringBuilder>
#include <QString>
#include <QSysInfo>
// QXmpp
#include <QXmppClient.h>
#include <QXmppConfiguration.h>
#include <QXmppPresence.h>
#include <QXmppUtils.h>
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

ClientWorker::ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QGuiApplication *app, QObject* parent)
	: QObject(parent), m_caches(caches), kaidan(kaidan), enableLogging(enableLogging), app(app)
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

	connect(client, &QXmppClient::presenceReceived,
	        caches->presCache, &PresenceCache::updatePresence);
	connect(client, &QXmppClient::disconnected,
	        caches->presCache, &PresenceCache::clear);

	connect(this, &ClientWorker::credentialsUpdated, this, &ClientWorker::setCredentials);

	// publish kaidan version
	QXmppVersionManager* versionManager = client->findExtension<QXmppVersionManager>();
	versionManager->setClientName(APPLICATION_DISPLAY_NAME);
	versionManager->setClientVersion(VERSION_STRING);
	versionManager->setClientOs(QSysInfo::prettyProductName());

	// Client State Indication
	connect(app, &QGuiApplication::applicationStateChanged, this, &ClientWorker::setCsiState);

	// account deletion
	connect(kaidan, &Kaidan::deleteAccountFromClient, this, &ClientWorker::deleteAccountFromClient);
	connect(kaidan, &Kaidan::deleteAccountFromClientAndServer, this, &ClientWorker::deleteAccountFromClientAndServer);
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->rosterDb(), &RosterDb::clearAll);
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->messageDb(), &MessageDb::removeAllMessages);

	connect(kaidan, &Kaidan::changePassword, this, &ClientWorker::changePassword);
	connect(kaidan, &Kaidan::changeDisplayName, this, &ClientWorker::changeDisplayName);
}

VCardManager *ClientWorker::getVCardManager() const
{
	return vCardManager;
}

ClientWorker::Caches *ClientWorker::caches() const
{
	return m_caches;
}

void ClientWorker::main()
{
	// Initialize the random number generator used by "qrand()" for QXmpp < 1.3.0 or QXmpp built with Qt < 5.10.
	// Please do not use that deprecated method for Kaidan.
	qsrand(time(nullptr));

	connect(client, &QXmppClient::stateChanged, kaidan, &Kaidan::setConnectionState);

	connect(client, &QXmppClient::connected, this, &ClientWorker::onConnected);
	connect(client, &QXmppClient::disconnected, this, &ClientWorker::onDisconnected);

	connect(client, &QXmppClient::error, this, &ClientWorker::onConnectionError);

	connect(this, &ClientWorker::connectRequested, this, &ClientWorker::xmppConnect);
	connect(this, &ClientWorker::registrationFormRequested, this, &ClientWorker::connectToRegister);
	connect(this, &ClientWorker::disconnectRequested, client, &QXmppClient::disconnectFromServer);
}

void ClientWorker::xmppConnect()
{
	QXmppConfiguration config;
	config.setJid(creds.jid);
	config.setResource(generateJidResourceWithRandomSuffix(creds.jidResourcePrefix));
	config.setPassword(creds.password);
	config.setAutoAcceptSubscriptions(false);

	connectToServer(config);
}

void ClientWorker::connectToServer(QXmppConfiguration config)
{
	if (client->state() != QXmppClient::DisconnectedState) {
		qWarning() << "[main] Tried to connect, even if still connected!" << "Requesting disconnect first and connecting then.";
		m_isReconnecting = true;
		m_configToBeUsedOnNextConnect = config;
		client->disconnectFromServer();
		return;
	}

	config.setJid(creds.jid);
	config.setStreamSecurityMode(QXmppConfiguration::TLSRequired);

	// on first try we must be sure that we connect successfully
	// otherwise this could end in a reconnection loop
	config.setAutoReconnectionEnabled(!creds.isFirstTry);

	// Reset the attribute for In-Band Registration support.
	// That is needed when the attribute was true after the last logout but the server disabled the support until the next login.
	// Without that reset, the attribute would stay "true".
	m_caches->serverFeaturesCache->setInBandRegistrationSupported(false);

	client->connectToServer(config);
}

void ClientWorker::connectToRegister()
{
	registrationManager->setRegisterOnConnectEnabled(true);
	connectToServer();
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
		client->disconnectFromServer();
	}
}

void ClientWorker::handleAccountDeletedFromServer()
{
	m_isAccountDeletedFromServer = true;
}

void ClientWorker::handleAccountDeletionFromServerFailed(QXmppStanza::Error error)
{
	emit kaidan->passiveNotificationRequested(tr("Your account could not be deleted from the server. Therefore, it was also not removed from this app: %1").arg(error.text()));

	m_isAccountToBeDeletedFromClientAndServer = false;

	if (!m_isClientConnectedBeforeAccountDeletionFromServer)
		client->disconnectFromServer();
}

void ClientWorker::changePassword(const QString &newPassword)
{
	if (client->isAuthenticated()) {
		registrationManager->changePassword(newPassword);
	} else {
		m_passwordToBeSetOnNextConnect = newPassword;
		xmppConnect();
	}
}

void ClientWorker::changeDisplayName(const QString &displayName)
{
	if (client->isAuthenticated()) {
		vCardManager->updateNickname(displayName);
	} else {
		m_displayNameToBeSetOnNextConnect = displayName;

		// The check is needed when this method is called during account registration to avoid connecting to the server before the registration is finished.
		// During registration, the display name is set first and the client connects to the server afterwards.
		// The client should only connect to the server during normal usage.
		if (!(creds.jid.isEmpty() || creds.password.isEmpty()))
			xmppConnect();
	}
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
	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_JID, creds.jid);
	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_PASSWD, QString::fromUtf8(creds.password.toUtf8().toBase64()));

	// If the account could not be deleted from the server because the client was disconnected, delete it now.
	if (m_isAccountToBeDeletedFromClientAndServer)
		registrationManager->deleteAccount();
	else
		// After the first login, the client should always try to reconnect automatically in case of a connection outage.
		client->configuration().setAutoReconnectionEnabled(true);

	// If the display name could not be changed on the server because the client was disconnected, change it now and disconnect again.
	if (!m_passwordToBeSetOnNextConnect.isEmpty()) {
		registrationManager->changePassword(m_passwordToBeSetOnNextConnect);
		m_passwordToBeSetOnNextConnect.clear();
		client->disconnectFromServer();
	}

	// If the display name could not be changed from the server because the client was disconnected, change it now.
	if (!m_displayNameToBeSetOnNextConnect.isEmpty()) {
		vCardManager->updateNickname(m_displayNameToBeSetOnNextConnect);
		m_displayNameToBeSetOnNextConnect.clear();
	}
}

void ClientWorker::onDisconnected()
{
	if (m_isReconnecting) {
		m_isReconnecting = false;
		connectToServer(m_configToBeUsedOnNextConnect);
		m_configToBeUsedOnNextConnect = {};
		return;
	}

	registrationManager->setRegisterOnConnectEnabled(false);

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
		qDebug() << "[client] XMPP stream error:" << xmppStreamError;
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

QString ClientWorker::generateJidResourceWithRandomSuffix(const QString jidResourcePrefix , unsigned int length) const
{
	return jidResourcePrefix % "." % QXmppUtils::generateStanzaHash(length);
}
