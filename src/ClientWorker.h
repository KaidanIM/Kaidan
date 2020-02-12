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
class QGuiApplication;
// QXmpp
#include <QXmppClient.h>
// Kaidan
#include "AvatarFileStorage.h"
#include "Database.h"
#include "Enums.h"
#include "Globals.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "RosterModel.h"
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
		Caches(Kaidan *kaidan, RosterDb *rosterDb, MessageDb *msgDb,
		       QObject *parent = nullptr)
		        : msgModel(new MessageModel(kaidan, msgDb, parent)),
		          rosterModel(new RosterModel(rosterDb, parent)),
			  avatarStorage(new AvatarFileStorage(parent)),
			  presCache(new PresenceCache(parent)),
			  transferCache(new TransferCache(parent)),
			  settings(new QSettings(APPLICATION_NAME, APPLICATION_NAME))
		{
			rosterModel->setMessageModel(msgModel);
		}

		~Caches()
		{
			delete msgModel;
			delete rosterModel;
			delete avatarStorage;
			delete presCache;
			delete transferCache;
			delete settings;
		}

		MessageModel *msgModel;
		RosterModel *rosterModel;
		AvatarFileStorage *avatarStorage;
		PresenceCache *presCache;
		TransferCache* transferCache;
		QSettings *settings;
	};

	struct Credentials {
		QString jid;
		QString jidResource;
		QString password;
		// if never connected successfully before with these credentials
		bool isFirstTry;
	};

	/**
	 * @param caches All caches running in the main thread for communication with the UI.
	 * @param kaidan Main back-end class, running in the main thread.
	 * @param enableLogging If logging of the XMPP stream should be done.
	 * @param app The QGuiApplication to determine if the window is active.
	 * @param parent Optional QObject-based parent.
	 */
	ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QGuiApplication *app,
	             QObject *parent = nullptr);

	VCardManager *getVCardManager() const;

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
		this->creds = creds;
	}

	/**
	 * Connects the client with the server.
	 */
	void xmppConnect();

signals:
	// emitted by 'Kaidan' to us:
	void connectRequested();
	void disconnectRequested();
	void credentialsUpdated(ClientWorker::Credentials creds);

	/**
	 * Emitted when the client failed to connect to the server.
	 *
	 * @param error new connection error
	 */
	void connectionErrorChanged(ClientWorker::ConnectionError error);

private slots:
	/**
	 * Notifys via signal that the client has connected.
	 */
	void onConnect();

	/**
	 * Sets a new connection error.
	 */
	void onConnectionError(QXmppClient::Error error);

	/**
	 * Uses the QGuiApplication state to reduce network traffic when window is minimized
	 */
	void setCsiState(Qt::ApplicationState state);

private:
	/**
	 * Generates a random alphanumeric string
	 *
	 * @param length The length of the generated string
	 */
	QString generateRandomString(unsigned int length = 4) const;

	Caches *caches;
	Kaidan *kaidan;
	QXmppClient *client;
	LogHandler *logger;
	Credentials creds;
	bool enableLogging;
	QGuiApplication *app;

	RegistrationManager *registrationManager;
	RosterManager *rosterManager;
	MessageHandler *msgHandler;
	DiscoveryManager *discoManager;
	VCardManager *vCardManager;
	UploadManager *uploadManager;
	DownloadManager *downloadManager;
};

#endif // CLIENTWORKER_H
