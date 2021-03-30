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

#include "ClientWorker.h"
// Qt
#include <QSettings>
// QXmpp
#include <QXmppUtils.h>
// Kaidan
#include "AccountManager.h"
#include "AvatarFileStorage.h"
#include "DiscoveryManager.h"
#include "DownloadManager.h"
#include "Enums.h"
#include "Globals.h"
#include "Kaidan.h"
#include "LogHandler.h"
#include "MessageDb.h"
#include "MessageHandler.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "RegistrationManager.h"
#include "RosterDb.h"
#include "RosterManager.h"
#include "RosterModel.h"
#include "ServerFeaturesCache.h"
#include "TransferCache.h"
#include "UploadManager.h"
#include "VCardManager.h"
#include "VersionManager.h"

ClientWorker::Caches::Caches(QObject *parent)
	: settings(new QSettings(APPLICATION_NAME, APPLICATION_NAME)),
	  accountManager(new AccountManager(settings, parent)),
	  msgModel(new MessageModel(parent)),
	  rosterModel(new RosterModel(parent)),
	  avatarStorage(new AvatarFileStorage(parent)),
	  serverFeaturesCache(new ServerFeaturesCache(parent)),
	  presCache(new PresenceCache(parent)),
	  transferCache(new TransferCache(parent))
{
	rosterModel->setMessageModel(msgModel);
}

ClientWorker::Caches::~Caches()
{
	delete settings;
}

ClientWorker::ClientWorker(Caches *caches, bool enableLogging, QObject* parent)
	: QObject(parent),
	  m_caches(caches),
	  m_client(new QXmppClient(this)),
	  m_logger(new LogHandler(m_client, enableLogging, this)),
	  m_enableLogging(enableLogging),
	  m_registrationManager(new RegistrationManager(this, m_client, m_caches->settings, this)),
	  m_vCardManager(new VCardManager(this, m_client, m_caches->avatarStorage, this)),
	  m_rosterManager(new RosterManager(m_client, m_caches->avatarStorage, m_vCardManager, this)),
	  m_messageHandler(new MessageHandler(this, m_client, this)),
	  m_discoveryManager(new DiscoveryManager(m_client, this)),
	  m_uploadManager(new UploadManager(m_client, m_rosterManager, this)),
	  m_downloadManager(new DownloadManager(caches->transferCache, this)),
	  m_versionManager(new VersionManager(m_client, this)),
	  m_isApplicationWindowActive(true)
{
	connect(m_client, &QXmppClient::connected, this, &ClientWorker::onConnected);
	connect(m_client, &QXmppClient::disconnected, this, &ClientWorker::onDisconnected);

	connect(m_client, &QXmppClient::stateChanged, this, &ClientWorker::onConnectionStateChanged);
	connect(m_client, &QXmppClient::error, this, &ClientWorker::onConnectionError);

	connect(Kaidan::instance(), &Kaidan::logInRequested, this, &ClientWorker::logIn);
	connect(Kaidan::instance(), &Kaidan::registrationFormRequested, this, &ClientWorker::connectToRegister);
	connect(Kaidan::instance(), &Kaidan::logOutRequested, this, &ClientWorker::logOut);

	// presence
	connect(m_client, &QXmppClient::presenceReceived, caches->presCache, &PresenceCache::updatePresence);
	connect(m_client, &QXmppClient::disconnected, caches->presCache, &PresenceCache::clear);

	// Inform the client worker when the application window becomes active or inactive.
	connect(Kaidan::instance(), &Kaidan::applicationWindowActiveChanged, this, &ClientWorker::setIsApplicationWindowActive);

	// Reduce the network traffic when the application window is inactive.
	connect(Kaidan::instance(), &Kaidan::applicationWindowActiveChanged, m_client, &QXmppClient::setActive);

	// account deletion
	connect(Kaidan::instance(), &Kaidan::deleteAccountFromClient, this, &ClientWorker::deleteAccountFromClient);
	connect(Kaidan::instance(), &Kaidan::deleteAccountFromClientAndServer, this, &ClientWorker::deleteAccountFromClientAndServer);
	connect(this, &ClientWorker::deleteAccountFromDatabase, RosterDb::instance(), &RosterDb::clearAll);
	connect(this, &ClientWorker::deleteAccountFromDatabase, MessageDb::instance(), &MessageDb::removeAllMessages);
}

void ClientWorker::initialize()
{
	// Initialize the random number generator used by "qrand()" for QXmpp < 1.3.0 or QXmpp
	// built with Qt < 5.10. Please do not use that deprecated method for Kaidan.
	qsrand(time(nullptr));
}

void ClientWorker::startTask(const std::function<void ()> task) {
	if (m_client->isAuthenticated()) {
		task();
	} else {
		m_pendingTasks << task;

		// The client should only log in during regular usage and not while initializing
		// the account (e.g. not when the user's display name is changed during
		// registration). The check is needed when this method is called before a login
		// with new credentials to avoid logging in to the server before all needed
		// credentials are available. That way, the variable can already be set locally
		// during account initialization (like during registration) and set on the server
		// after the first login.
		if (!AccountManager::instance()->hasNewCredentials() && m_client->state() != QXmppClient::ConnectingState)
			logIn();
	}
}

void ClientWorker::finishTask()
{
	// If m_activeTasks > 0, there are still running tasks.
	// If m_activeTasks = 0, all tasks are finished (the tasks may have finished directly).
	if (m_activeTasks > 0 && --m_activeTasks == 0 && !AccountManager::instance()->hasNewCredentials())
		logOut();
}

void ClientWorker::logIn()
{
	if (!m_isFirstLoginAfterStart || m_caches->settings->value(KAIDAN_SETTINGS_AUTH_ONLINE, true).toBool()) {
		// Store the latest online state which is restored when opening Kaidan again after closing.
		m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_ONLINE, true);

		QXmppConfiguration config;
		config.setResource(AccountManager::instance()->jidResource());
		config.setPassword(AccountManager::instance()->password());
		config.setAutoAcceptSubscriptions(false);

		connectToServer(config);
	}

	m_isFirstLoginAfterStart = false;
}

void ClientWorker::connectToRegister()
{
	AccountManager::instance()->setHasNewCredentials(true);
	m_registrationManager->setRegisterOnConnectEnabled(true);
	connectToServer();
}

void ClientWorker::connectToServer(QXmppConfiguration config)
{
	switch (m_client->state()) {
	case QXmppClient::ConnectingState:
		qDebug() << "[main] Tried to connect even if already connecting! Nothing is done.";
		break;
	case QXmppClient::ConnectedState:
		qDebug() << "[main] Tried to connect even if already connected! Disconnecting first and connecting afterwards.";
		m_isReconnecting = true;
		m_configToBeUsedOnNextConnect = config;
		logOut();
		break;
	case QXmppClient::DisconnectedState:
		config.setJid(AccountManager::instance()->jid());
		config.setStreamSecurityMode(QXmppConfiguration::TLSRequired);

		auto host = AccountManager::instance()->host();
		if (!host.isEmpty())
			config.setHost(host);

		auto port = AccountManager::instance()->port();
		if (port != NON_CUSTOM_PORT) {
			config.setPort(port);

			// Set the JID's domain part as the host if no custom host is set.
			if (host.isEmpty())
				config.setHost(QXmppUtils::jidToDomain(AccountManager::instance()->jid()));
		}

		// Disable the automatic reconnection in case this connection attempt is not
		// successful. Otherwise, this could result in a reconnection loop. It is enabled
		// again after a successful login.
		config.setAutoReconnectionEnabled(false);

		// Reset the attribute for In-Band Registration support. That is needed when the
		// attribute was true until the last logout but the server disabled the support
		// afterwards. Without that reset, the attribute would stay "true" even if the
		// server did not support it anymore.
		m_caches->serverFeaturesCache->setInBandRegistrationSupported(false);

		m_client->connectToServer(config);
	}
}

void ClientWorker::logOut(bool isApplicationBeingClosed)
{
	// Store the latest online state which is restored when opening Kaidan again after closing.
	if (!isApplicationBeingClosed)
		m_caches->settings->setValue(KAIDAN_SETTINGS_AUTH_ONLINE, false);

	switch (m_client->state()) {
	case QXmppClient::DisconnectedState:
		break;
	case QXmppClient::ConnectingState:
		qDebug() << "[main] Tried to disconnect even if still connecting! Waiting for connecting to succeed and disconnect afterwards.";
		m_isDisconnecting = true;
		break;
	case QXmppClient::ConnectedState:
		if (AccountManager::instance()->hasNewCredentials())
			AccountManager::instance()->setHasNewCredentials(false);

		m_client->disconnectFromServer();
	}
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
		logIn();
	}
}

void ClientWorker::deleteAccountFromClient()
{
	// If the client is already disconnected, delete the account directly from the client.
	// Otherwise, disconnect first and delete the account afterwards.
	if (!m_client->isAuthenticated()) {
		emit deleteAccountFromDatabase();
		AccountManager::instance()->deleteSettings();
		AccountManager::instance()->deleteCredentials();
		m_isAccountToBeDeletedFromClient = false;
	} else {
		m_isAccountToBeDeletedFromClient = true;
		logOut();
	}
}

void ClientWorker::handleAccountDeletedFromServer()
{
	m_isAccountDeletedFromServer = true;
}

void ClientWorker::handleAccountDeletionFromServerFailed(const QXmppStanza::Error &error)
{
	emit Kaidan::instance()->passiveNotificationRequested(tr("Your account could not be deleted from the server. Therefore, it was also not removed from this app: %1").arg(error.text()));

	m_isAccountToBeDeletedFromClientAndServer = false;

	if (!m_isClientConnectedBeforeAccountDeletionFromServer)
		logOut();
}

bool ClientWorker::isApplicationWindowActive() const
{
	return m_isApplicationWindowActive;
}

void ClientWorker::onConnected()
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Connected successfully to server";

	// If there was an error before, notify about its absence.
	emit connectionErrorChanged(ClientWorker::NoError);

	// If the account could not be deleted from the server because the client was
	// disconnected, delete it now.
	if (m_isAccountToBeDeletedFromClientAndServer) {
		m_registrationManager->deleteAccount();
		return;
	}

	// Try to complete pending tasks which could not be completed while the client was
	// offline and skip further normal tasks if at least one was completed after a login
	// with old credentials.
	if (startPendingTasks())
		return;

	// If the client was connecting during a disconnection attempt, disconnect it now.
	if (m_isDisconnecting) {
		m_isDisconnecting = false;
		logOut();
		return;
	}

	// The following tasks are only done after a login with new credentials.
	if (AccountManager::instance()->hasNewCredentials()) {
		emit loggedInWithNewCredentials();

		// Store the valid settings.
		AccountManager::instance()->storeCredentials();
	}

	// Enable auto reconnection so that the client is always trying to reconnect
	// automatically in case of a connection outage.
	m_client->configuration().setAutoReconnectionEnabled(true);

	MessageModel::instance()->sendPendingMessages();
}

void ClientWorker::onDisconnected()
{
	if (m_isReconnecting) {
		m_isReconnecting = false;
		connectToServer(m_configToBeUsedOnNextConnect);
		m_configToBeUsedOnNextConnect = {};
		return;
	}

	// Delete the account from the client if the client was connected and had to
	// disconnect first or if the account was deleted from the server.
	if (m_isAccountToBeDeletedFromClient || (m_isAccountToBeDeletedFromClientAndServer && m_isAccountDeletedFromServer)) {
		m_isAccountToBeDeletedFromClientAndServer = false;
		m_isAccountDeletedFromServer = false;

		deleteAccountFromClient();
	}
}

void ClientWorker::onConnectionStateChanged(QXmppClient::State connectionState)
{
	emit connectionStateChanged(Enums::ConnectionState(connectionState));
}

void ClientWorker::onConnectionError(QXmppClient::Error error)
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Disconnected:" << error;

	QXmppStanza::Error::Condition xmppStreamError;
	QAbstractSocket::SocketError socketError;

	switch (error) {
	case QXmppClient::NoError:
		Q_UNREACHABLE();
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

bool ClientWorker::startPendingTasks()
{
	bool isBusy = false;

	while (!m_pendingTasks.isEmpty()) {
		m_pendingTasks.takeFirst()();
		m_activeTasks++;

		isBusy = true;
	}

	return !AccountManager::instance()->hasNewCredentials() && isBusy;
}
