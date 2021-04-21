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

#pragma once

// Qt
// QXmpp
#include <QXmppClient.h>
// Kaidan
#include "Enums.h"

class AccountManager;
class AvatarFileStorage;
class DiscoveryManager;
class DownloadManager;
class LogHandler;
class MessageHandler;
class MessageModel;
class PresenceCache;
class RegistrationManager;
class RosterManager;
class RosterModel;
class ServerFeaturesCache;
class TransferCache;
class UploadManager;
class VCardCache;
class VCardManager;
class VersionManager;
class QSettings;

/**
 * The ClientWorker is used as a QObject-based worker on the ClientThread.
 */
class ClientWorker : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RegistrationManager* registrationManager READ registrationManager CONSTANT)
	Q_PROPERTY(VCardManager* vCardManager READ vCardManager CONSTANT)
	Q_PROPERTY(RosterManager* rosterManager READ rosterManager CONSTANT)
	Q_PROPERTY(MessageHandler* messageHandler READ messageHandler CONSTANT)
	Q_PROPERTY(DiscoveryManager* discoveryManager READ discoveryManager CONSTANT)
	Q_PROPERTY(UploadManager* uploadManager READ uploadManager CONSTANT)
	Q_PROPERTY(DownloadManager* downloadManager READ downloadManager CONSTANT)
	Q_PROPERTY(VersionManager* versionManager READ versionManager CONSTANT)

public:
	/**
	 * enumeration of possible connection errors
	 */
	enum ConnectionError {
		NoError,
		AuthenticationFailed,
		NotConnected,
		TlsFailed,
		TlsNotAvailable,
		DnsError,
		ConnectionRefused,
		NoSupportedAuth,
		KeepAliveError,
		NoNetworkPermission,
		RegistrationUnsupported
	};
	Q_ENUM(ConnectionError)

	struct Caches {
		Caches(QObject *parent = nullptr);
		~Caches();

		QSettings *settings;
		VCardCache *vCardCache;
		AccountManager *accountManager;
		MessageModel *msgModel;
		RosterModel *rosterModel;
		AvatarFileStorage *avatarStorage;
		ServerFeaturesCache *serverFeaturesCache;
		PresenceCache *presCache;
		TransferCache* transferCache;
	};

	/**
	 * @param caches All caches running in the main thread for communication with the UI.
	 * @param enableLogging If logging of the XMPP stream should be done.
	 * @param parent Optional QObject-based parent.
	 */
	ClientWorker(Caches *caches, bool enableLogging, QObject *parent = nullptr);

	RegistrationManager *registrationManager() const
	{
		return m_registrationManager;
	}

	VCardManager *vCardManager() const
	{
		return m_vCardManager;
	}

	RosterManager *rosterManager() const
	{
		return m_rosterManager;
	}

	MessageHandler *messageHandler() const
	{
		return m_messageHandler;
	}

	DiscoveryManager *discoveryManager() const
	{
		return m_discoveryManager;
	}

	UploadManager *uploadManager() const
	{
		return m_uploadManager;
	}

	DownloadManager *downloadManager() const
	{
		return m_downloadManager;
	}

	VersionManager *versionManager() const
	{
		return m_versionManager;
	}

	Caches *caches() const
	{
		return m_caches;
	}

	/**
	 * Starts or enqueues a task which will be executed after successful login (e.g. a
	 * nickname change).
	 *
	 * This method is called by managers which must call "finishTask()" as soon as the
	 * task is completed.
	 *
	 * If the user is logged out when this method is called, a login is triggered, the
	 * task is started and a logout is triggered afterwards. However, if this method is
	 * called before a login with new credentials (e.g. during account registration), the
	 * task is started after the subsequent login.
	 *
	 * @param task task which is run directly if the user is logged in or enqueued to be
	 * run after an automatic login
	 */
	void startTask(const std::function<void ()> task);

	/**
	 * Finishes a task started by "startTask()".
	 *
	 * This must be called after a possible completion of a pending task.
	 *
	 * A logout is triggered when this method is called after the second login with the
	 * same credentials or later. That means, a logout is not triggered after a login with
	 * new credentials (e.g. after a registration).
	 */
	void finishTask();

public slots:
	/**
	 * Connects to the server and logs in with all needed configuration variables.
	 */
	void logIn();

	/**
	 * Connects to the server and requests a data form for account registration.
	 */
	void connectToRegister();

	/**
	 * Connects to the server with a minimal configuration.
	 *
	 * Some additional configuration variables can be set by passing a configuration.
	 *
	 * @param config configuration with additional variables for connecting to the server
	 * or nothing if only the minimal configuration should be used
	 */
	void connectToServer(QXmppConfiguration config = QXmppConfiguration());

	/**
	 * Logs out of the server if the client is not already logged out.
	 *
	 * @param isApplicationBeingClosed true if the application will be terminated directly after logging out, false otherwise
	 */
	void logOut(bool isApplicationBeingClosed = false);

	/**
	 * Deletes the account data from the client and server.
	 */
	void deleteAccountFromClientAndServer();

	/**
	 * Deletes the account data from the configuration file and database.
	 */
	void deleteAccountFromClient();

	/**
	 * Called when the account is deleted from the server.
	 */
	void handleAccountDeletedFromServer();

	/**
	 * Called when the account could not be deleted from the server.
	 *
	 * @param error error of the failed account deletion
	 */
	void handleAccountDeletionFromServerFailed(const QXmppStanza::Error &error);

signals:
	/**
	 * Emitted when an authenticated connection to the server is established with new
	 * credentials for the first time.
	 *
	 * The client will be in connected state when this is emitted.
	 */
	void loggedInWithNewCredentials();

	/**
	 * Emitted when the client's connection state changed.
	 *
	 * @param connectionState new connection state
	 */
	void connectionStateChanged(Enums::ConnectionState connectionState);

	/**
	 * Emitted when the client failed to connect to the server.
	 *
	 * @param error new connection error
	 */
	void connectionErrorChanged(ClientWorker::ConnectionError error);

private slots:
	/**
	 * Called when an authenticated connection to the server is established.
	 */
	void onConnected();

	/**
	 * Called when the connection to the server is closed.
	 */
	void onDisconnected();

	/**
	 * Handles the change of the connection state.
	 *
	 * @param connectionState new connection state
	 */
	void onConnectionStateChanged(QXmppClient::State connectionState);

	/**
	 * Handles a connection error.
	 *
	 * @param error new connection error
	 */
	void onConnectionError(QXmppClient::Error error);

private:
	/**
	 * Starts a pending (enqueued) task (e.g. a password change) if the variable (e.g. a
	 * password) could not be changed on the server before because the client was not
	 * logged in.
	 *
	 * @return true if at least one pending task is started on the second login with the
	 * same credentials or later, otherwise false
	 */
	bool startPendingTasks();

	Caches *m_caches;
	QXmppClient *m_client;
	LogHandler *m_logger;
	bool m_enableLogging;

	RegistrationManager *const m_registrationManager;
	VCardManager *const m_vCardManager;
	RosterManager *const m_rosterManager;
	MessageHandler *const m_messageHandler;
	DiscoveryManager *const m_discoveryManager;
	UploadManager *const m_uploadManager;
	DownloadManager *const m_downloadManager;
	VersionManager *const m_versionManager;

	QList<std::function<void ()>> m_pendingTasks;
	uint m_activeTasks = 0;

	bool m_isFirstLoginAfterStart = true;
	bool m_isReconnecting = false;
	bool m_isDisconnecting = false;
	QXmppConfiguration m_configToBeUsedOnNextConnect;

	// These variables are used for checking the state of an ongoing account deletion.
	bool m_isAccountToBeDeletedFromClient = false;
	bool m_isAccountToBeDeletedFromClientAndServer = false;
	bool m_isAccountDeletedFromServer = false;
	bool m_isClientConnectedBeforeAccountDeletionFromServer = true;
};
