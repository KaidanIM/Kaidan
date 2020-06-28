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

ClientWorker::ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QObject* parent)
	: QObject(parent), m_caches(caches), m_kaidan(kaidan), m_enableLogging(enableLogging)
{
	m_client = new QXmppClient(this);
	m_logger = new LogHandler(m_client, this);
	m_logger->enableLogging(enableLogging);
	m_vCardManager = new VCardManager(m_client, caches->avatarStorage, this);
	m_registrationManager = new RegistrationManager(kaidan, this, m_client, caches->settings);
	m_rosterManager = new RosterManager(kaidan, m_client,  caches->rosterModel,
	                                    caches->avatarStorage, m_vCardManager, this);
	m_msgHandler = new MessageHandler(kaidan, this, m_client, caches->msgModel);
	m_discoManager = new DiscoveryManager(m_client, this);
	m_uploadManager = new UploadManager(m_client, m_rosterManager, this);
	m_downloadManager = new DownloadManager(kaidan, caches->transferCache,
	                                        caches->msgModel, this);

	connect(m_client, &QXmppClient::presenceReceived,
	        caches->presCache, &PresenceCache::updatePresence);
	connect(m_client, &QXmppClient::disconnected,
	        caches->presCache, &PresenceCache::clear);

	connect(this, &ClientWorker::credentialsUpdated, this, &ClientWorker::setCredentials);

	// publish kaidan version
	QXmppVersionManager* versionManager = m_client->findExtension<QXmppVersionManager>();
	versionManager->setClientName(APPLICATION_DISPLAY_NAME);
	versionManager->setClientVersion(VERSION_STRING);
	versionManager->setClientOs(QSysInfo::prettyProductName());

	// Inform the client worker when the application window becomes active or inactive.
	connect(kaidan, &Kaidan::applicationWindowActiveChanged, this, &ClientWorker::setIsApplicationWindowActive);

	// Reduce the network traffic when the application window is inactive.
	connect(kaidan, &Kaidan::applicationWindowActiveChanged, m_client, &QXmppClient::setActive);

	// account deletion
	connect(kaidan, &Kaidan::deleteAccountFromClient, this, &ClientWorker::deleteAccountFromClient);
	connect(kaidan, &Kaidan::deleteAccountFromClientAndServer, this, &ClientWorker::deleteAccountFromClientAndServer);
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->rosterDb(), &RosterDb::clearAll);
	connect(this, &ClientWorker::deleteAccountFromDatabase, kaidan->messageDb(), &MessageDb::removeAllMessages);

	connect(kaidan, &Kaidan::changePassword, this, &ClientWorker::changePassword);
	connect(kaidan, &Kaidan::changeDisplayName, this, &ClientWorker::changeDisplayName);
}

VCardManager *ClientWorker::vCardManager() const
{
	return m_vCardManager;
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

	connect(m_client, &QXmppClient::stateChanged, m_kaidan, &Kaidan::setConnectionState);

	connect(m_client, &QXmppClient::connected, this, &ClientWorker::onConnected);
	connect(m_client, &QXmppClient::disconnected, this, &ClientWorker::onDisconnected);

	connect(m_client, &QXmppClient::error, this, &ClientWorker::onConnectionError);

	connect(this, &ClientWorker::connectRequested, this, &ClientWorker::xmppConnect);
	connect(this, &ClientWorker::registrationFormRequested, this, &ClientWorker::connectToRegister);
	connect(this, &ClientWorker::disconnectRequested, m_client, &QXmppClient::disconnectFromServer);
}

void ClientWorker::xmppConnect()
{
	QXmppConfiguration config;
	config.setJid(m_creds.jid);
	config.setResource(generateJidResourceWithRandomSuffix(m_creds.jidResourcePrefix));
	config.setPassword(m_creds.password);
	config.setAutoAcceptSubscriptions(false);

	connectToServer(config);
}

void ClientWorker::connectToServer(QXmppConfiguration config)
{
	if (m_client->state() != QXmppClient::DisconnectedState) {
		qWarning() << "[main] Tried to connect, even if still connected!" << "Requesting disconnect first and connecting then.";
		m_isReconnecting = true;
		m_configToBeUsedOnNextConnect = config;
		m_client->disconnectFromServer();
		return;
	}

	config.setJid(m_creds.jid);
	config.setStreamSecurityMode(QXmppConfiguration::TLSRequired);

	// on first try we must be sure that we connect successfully
	// otherwise this could end in a reconnection loop
	config.setAutoReconnectionEnabled(!m_creds.isFirstTry);

	// Reset the attribute for In-Band Registration support.
	// That is needed when the attribute was true after the last logout but the server disabled the support until the next login.
	// Without that reset, the attribute would stay "true".
	m_caches->serverFeaturesCache->setInBandRegistrationSupported(false);

	m_client->connectToServer(config);
}

void ClientWorker::connectToRegister()
{
	m_registrationManager->setRegisterOnConnectEnabled(true);
	connectToServer();
}

void ClientWorker::deleteAccountFromClientAndServer()
{
	m_isAccountToBeDeletedFromClientAndServer = true;

	// If the client is already connected, delete the account directly from the server.
	// Otherwise, connect first and delete the account afterwards.
	if (m_client->isAuthenticated()) {
		m_registrationManager->deleteAccount();
	} else {
		m_isClientConnectedBeforeAccountDeletionFromServer = false;
		xmppConnect();
	}
}

void ClientWorker::deleteAccountFromClient()
{
	// If the client is already disconnected, delete the account directly from the client.
	// Otherwise, disconnect first and delete the account afterwards.
	if (!m_client->isAuthenticated()) {
		emit deleteAccountFromDatabase();
		m_kaidan->deleteCredentials();
		m_isAccountToBeDeletedFromClient = false;
	} else {
		m_isAccountToBeDeletedFromClient = true;
		m_client->disconnectFromServer();
	}
}

void ClientWorker::handleAccountDeletedFromServer()
{
	m_isAccountDeletedFromServer = true;
}

void ClientWorker::handleAccountDeletionFromServerFailed(QXmppStanza::Error error)
{
	emit m_kaidan->passiveNotificationRequested(tr("Your account could not be deleted from the server. Therefore, it was also not removed from this app: %1").arg(error.text()));

	m_isAccountToBeDeletedFromClientAndServer = false;

	if (!m_isClientConnectedBeforeAccountDeletionFromServer)
		m_client->disconnectFromServer();
}

void ClientWorker::changePassword(const QString &newPassword)
{
	if (m_client->isAuthenticated()) {
		m_registrationManager->changePassword(newPassword);
	} else {
		m_passwordToBeSetOnNextConnect = newPassword;
		xmppConnect();
	}
}

void ClientWorker::changeDisplayName(const QString &displayName)
{
	if (m_client->isAuthenticated()) {
		m_vCardManager->updateNickname(displayName);
	} else {
		m_displayNameToBeSetOnNextConnect = displayName;

		// The check is needed when this method is called during account registration to avoid connecting to the server before the registration is finished.
		// During registration, the display name is set first and the client connects to the server afterwards.
		// The client should only connect to the server during normal usage.
		if (!(m_creds.jid.isEmpty() || m_creds.password.isEmpty()))
			xmppConnect();
	}
}

bool ClientWorker::isApplicationWindowActive() const
{
	return m_isApplicationWindowActive;
}

void ClientWorker::onConnected()
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Connected successfully to server";

	// Emit signal, that logging in with these credentials has worked for the first time
	if (m_creds.isFirstTry)
		emit m_kaidan->loggedInWithNewCredentials();

	// accept credentials and save them
	m_creds.isFirstTry = false;
	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_JID, m_creds.jid);
	m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_PASSWD, QString::fromUtf8(m_creds.password.toUtf8().toBase64()));

	// If the account could not be deleted from the server because the client was disconnected, delete it now.
	if (m_isAccountToBeDeletedFromClientAndServer) {
		m_registrationManager->deleteAccount();
		return;
	}

	// After the first login, the client should always try to reconnect automatically in case of a connection outage.
	m_client->configuration().setAutoReconnectionEnabled(true);

	// If the display name could not be changed on the server because the client was disconnected, change it now and disconnect again.
	if (!m_passwordToBeSetOnNextConnect.isEmpty()) {
		m_registrationManager->changePassword(m_passwordToBeSetOnNextConnect);
		m_passwordToBeSetOnNextConnect.clear();
		m_client->disconnectFromServer();
		return;
	}

	// If the display name could not be changed from the server because the client was disconnected, change it now.
	if (!m_displayNameToBeSetOnNextConnect.isEmpty()) {
		m_vCardManager->updateNickname(m_displayNameToBeSetOnNextConnect);
		m_displayNameToBeSetOnNextConnect.clear();
	}

	emit m_caches->msgModel->sendPendingMessages();
}

void ClientWorker::onDisconnected()
{
	if (m_isReconnecting) {
		m_isReconnecting = false;
		connectToServer(m_configToBeUsedOnNextConnect);
		m_configToBeUsedOnNextConnect = {};
		return;
	}

	m_registrationManager->setRegisterOnConnectEnabled(false);

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
		xmppStreamError = m_client->xmppStreamError();
		qDebug() << "[client] XMPP stream error:" << xmppStreamError;
		if (xmppStreamError == QXmppStanza::Error::NotAuthorized) {
			emit connectionErrorChanged(ClientWorker::AuthenticationFailed);
		} else {
			emit connectionErrorChanged(ClientWorker::NotConnected);
		}
		break;
	case QXmppClient::SocketError:
		socketError = m_client->socketError();
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

void ClientWorker::setIsApplicationWindowActive(bool active)
{
	m_isApplicationWindowActive = active;
}

QString ClientWorker::generateJidResourceWithRandomSuffix(const QString jidResourcePrefix , unsigned int length) const
{
	return jidResourcePrefix % "." % QXmppUtils::generateStanzaHash(length);
}
