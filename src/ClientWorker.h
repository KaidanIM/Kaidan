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

#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

// Qt
#include <QObject>
#include <QSettings>
#include <QTimer>
// QXmpp
#include <QXmppClient.h>
// Kaidan
#include "AvatarFileStorage.h"
#include "BitsOfBinaryImageProvider.h"
#include "Database.h"
#include "Enums.h"
#include "Globals.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "RosterModel.h"
#include "ServerFeaturesCache.h"
#include "TransferCache.h"
class LogHandler;
class Kaidan;
class ClientWorker;
class RegistrationManager;
class RosterManager;
class MessageHandler;
class DiscoveryManager;
class VCardManager;
class UploadManager;
class DownloadManager;

/**
 * The ClientWorker is used as a QObject-based worker on the ClientThread.
 */
class ClientWorker : public QObject
{
	Q_OBJECT

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
		Caches(Kaidan *kaidan, RosterDb *rosterDb, MessageDb *msgDb, QObject *parent = nullptr)
		        : msgModel(new MessageModel(kaidan, msgDb, parent)),
		          rosterModel(new RosterModel(rosterDb, parent)),
		          avatarStorage(new AvatarFileStorage(parent)),
		          serverFeaturesCache(new ServerFeaturesCache(parent)),
		          presCache(new PresenceCache(parent)),
		          transferCache(new TransferCache(parent)),
		          settings(new QSettings(APPLICATION_NAME, APPLICATION_NAME))
		{
			rosterModel->setMessageModel(msgModel);
		}

		~Caches()
		{
			delete settings;
		}

		MessageModel *msgModel;
		RosterModel *rosterModel;
		AvatarFileStorage *avatarStorage;
		ServerFeaturesCache *serverFeaturesCache;
		PresenceCache *presCache;
		TransferCache* transferCache;
		QSettings *settings;
	};

	struct Credentials {
		QString jid;
		QString jidResourcePrefix;
		QString password;
		// if never connected successfully before with these credentials
		bool isFirstTry;
	};

	/**
	 * @param caches All caches running in the main thread for communication with the UI.
	 * @param kaidan Main back-end class, running in the main thread.
	 * @param enableLogging If logging of the XMPP stream should be done.
	 * @param parent Optional QObject-based parent.
	 */
	ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QObject *parent = nullptr);

	VCardManager *vCardManager() const;

	/**
	 * Returns all models and caches.
	 */
	Caches *caches() const;

public slots:
	/**
	 * Main function of the client thread
	 */
	void main();

	/**
	 * Sets the new credentials for next connect.
	 *
	 * @param creds The new credentials for the next connect
	 */
	void setCredentials(ClientWorker::Credentials creds)
	{
		m_creds = creds;
	}

	/**
	 * Connects to the server and logs in with all needed configuration variables.
	 */
	void xmppConnect();

	/**
	 * Connects to the server with a minimal configuration and adds additional variables to it before connecting if a configuration is passed.
	 *
	 * @param config
	 */
	void connectToServer(QXmppConfiguration config = QXmppConfiguration());

	/**
	 * Connects to the server and requests a data form for account registration.
	 */
	void connectToRegister();

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
	void handleAccountDeletionFromServerFailed(QXmppStanza::Error error);

	/**
	 * Changes the user's password.
	 *
	 * @param newPassword new password to set
	 */
	void changePassword(const QString &newPassword);

	/**
	 * Changes the user's display name.
	 *
	 * If the user is not logged in during account registration, the display name is changed on the next login.
	 *
	 * @param displayName new name that is shown to contacts
	 */
	void changeDisplayName(const QString &displayName);

	/**
	 * Returns whether the application window is active.
	 *
	 * The application window is active when it is in the foreground and focused.
	 */
	bool isApplicationWindowActive() const;

signals:
	/**
	 * Requests to show a notification for a chat message via the system's notification channel.
	 *
	 * @param senderJid JID of the message's sender
	 * @param senderName name of the message's sender
	 * @param message message to show
	 */
	void showMessageNotificationRequested(const QString &senderJid, const QString &senderName, const QString &message);

	// Those signals are emitted by Kaidan.cpp and are used by this class.
	void connectRequested();
	void disconnectRequested();
	void credentialsUpdated(ClientWorker::Credentials m_creds);
	void registrationFormRequested();

	/**
	 * Emitted when the client failed to connect to the server.
	 *
	 * @param error new connection error
	 */
	void connectionErrorChanged(ClientWorker::ConnectionError error);

	/**
	 * Deletes data related to the current account (messages, contacts etc.) from the database.
	 */
	void deleteAccountFromDatabase();

private slots:
	/**
	 * Sets the value to know whether the application window is active.
	 *
	 * @param active true if the application window is active, false otherwise
	 */
	void setIsApplicationWindowActive(bool active);

	/**
	 * Called when an authenticated connection to the server is established.
	 */
	void onConnected();

	/**
	 * Called when the connection to the server is closed.
	 */
	void onDisconnected();

	/**
	 * Sets a new connection error.
	 */
	void onConnectionError(QXmppClient::Error error);

private:
	/**
	 * Generates the resource part of a JID with a suffix consisting of a dot followed by random alphanumeric characters.
	 *
	 * @param length number of random alphanumeric characters the suffix should consist of after the dot
	 */
	QString generateJidResourceWithRandomSuffix(const QString jidResourcePrefix, unsigned int length = 4) const;

	Caches *m_caches;

	Kaidan *m_kaidan;
	QXmppClient *m_client;
	LogHandler *m_logger;
	Credentials m_creds;
	bool m_enableLogging;

	RegistrationManager *m_registrationManager;
	RosterManager *m_rosterManager;
	MessageHandler *m_msgHandler;
	DiscoveryManager *m_discoManager;
	VCardManager *m_vCardManager;
	UploadManager *m_uploadManager;
	DownloadManager *m_downloadManager;

	bool m_isApplicationWindowActive;
	bool m_isReconnecting = false;
	QXmppConfiguration m_configToBeUsedOnNextConnect;

	// These variables are used for checking the state of an ongoing account deletion.
	bool m_isAccountToBeDeletedFromClient = false;
	bool m_isAccountToBeDeletedFromClientAndServer = false;
	bool m_isAccountDeletedFromServer = false;
	bool m_isClientConnectedBeforeAccountDeletionFromServer = true;

	QString m_passwordToBeSetOnNextConnect;
	QString m_displayNameToBeSetOnNextConnect;
};

#endif // CLIENTWORKER_H
