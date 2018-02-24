/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

// gloox
#include <gloox/client.h>
// Qt
#include <QMutex>
#include <QThread>
#include <QTimer>
// Kaidan
#include "Enums.h"

namespace gloox {
	class Client;
}
class ClientWorker;
class RosterManager;
class RosterModel;
class MessageSessionHandler;
class MessageModel;
class AvatarFileStorage;
class PresenceHandler;
class ServiceDiscoveryManager;
class VCardManager;
class XmlLogHandler;
class QSettings;
class QGuiApplication;

using namespace Enums;

/**
 * @class KaidanClient Needed to replace server after first connection
 */
class GlooxClient : public gloox::Client
{
public:
	GlooxClient(gloox::JID jid, std::string password, int port = -1)
	            : gloox::Client(jid, password, port)
	{
	}

	void setJidServer(const std::string &server)
	{
		m_jid.setServer(server);
	}
};

/**
 * @class ClientThread A class controlling the thread used for the XMPP
 * connection.
 * 
 * @see ClientWorker
 */
class ClientThread : public QThread
{
	Q_OBJECT

	friend class ClientWorker; // can access private vars

public:
	struct Credentials {
		QString jid;
		QString jidResource;
		QString password;
		// if never connected successfully before with these credentials
		bool isFirstTry;
	};

	/**
	 * @param rosterModel The roster model connected to the database
	 * @param messageModel The message model connected to the database
	 * @param avatarStorage The storage class for caching user avatars
	 * @param creds The used credentials for connection
	 * @param settings Settings used for saving creds. on successful connect
	 * @param parent Optional QObject-based parent
	 */
	ClientThread(RosterModel *rosterModel, MessageModel *messageModel,
	             AvatarFileStorage *avatarStorage, Credentials creds,
	             QSettings *settings, QGuiApplication *app,
	             QObject *parent = nullptr);

	/*
	 * Will exit the event loop and waits until thread finishes and then
	 * destroys the object
	 */
	~ClientThread();

	/**
	 * Sets the new credentials for next connect.
	 * 
	 * @param jid The users's JID
	 * @param jidResource The resource/device name used for connecting
	 * @param password The user's password
	 */
	void setCredentials(Credentials creds);

	/**
	 * Applys filters to the database for showing the correct chat.
	 */
	void setCurrentChatPartner(QString *jid);

	/**
	 * Returns if connection state is connected.
	 */
	bool isConnected()
	{
		return connState == ConnectionState::StateConnected;
	}

	/**
	 * Returns the current connection state
	 * 
	 * @see Enums::ConnectionState
	 */
	ConnectionState getConnectionState()
	{
		return connState;
	}

	/**
	 * Returns the last error that caused a disconnection.
	 */
	DisconnReason getConnectionError()
	{
		return (DisconnReason) connError;
	}

protected:
	/**
	 * The thread's main function which will initialize the client and it's
	 * subcomponents.
	 */
	void run() override;

signals:
	/**
	 * Emitted when the connection state has changed.
	 */
	void connectionStateChanged(ConnectionState state);
	
	/**
	 * Emitted when the client failed to connect giving the reason of it.
	 */
	void disconnReasonChanged(DisconnReason reason);
	
	/**
	 * Emit to start connecting on the client's thread.
	 */
	void connectRequested();
	
	/**
	 * Emit to start the disconnection on the client's thread.
	 */
	void disconnectRequested();
	
	/**
	 * Emit to send a message to a chat partner.
	 */
	void sendMessageRequested(QString toJid, QString message);
	
	/**
	 * Emit to add a new contact to the roster.
	 */
	void addContactRequested(QString jid, QString name);

	/**
	 * Emit to remove a contact from the roster.
	 */
	void removeContactRequested(QString jid);

	/**
	 * Emit to stop the work timer (needed because it has to be killed on
	 * its thread).
	 */
	void stopWorkTimerRequested();

	/**
	 * Emit to get new credentials from QML
	 */
	void newCredentialsNeeded();

	/**
	 * Emitted, when logging in with new credentials worked
	 */
	void logInWorked();

private:
	/**
	 * Emits the signal for a given connection state.
	 * 
	 * @param state The new connection state
	 */
	void setConnectionState(ConnectionState state);

	/**
	 * Sets the new error as Enums::DisconnReason and emits the according
	 * signal
	 * 
	 * @param error The last connection error
	 */
	void setConnectionError(gloox::ConnectionError error);

	RosterModel *rosterModel;
	MessageModel *messageModel;
	AvatarFileStorage *avatarStorage;
	Credentials creds;

	GlooxClient *client;
	ClientWorker *worker;
	RosterManager *rosterManager;
	MessageSessionHandler *messageSessionHandler;
	PresenceHandler *presenceHandler;
	ServiceDiscoveryManager *serviceDiscoveryManager;
	VCardManager *vCardManager;
	XmlLogHandler *xmlLogHandler;
	QSettings *settings;
	QMutex mutex;
	QTimer workTimer;
	ConnectionState connState = ConnectionState::StateNone;
	gloox::ConnectionError connError = gloox::ConnNoError;
};

#endif // CLIENTTHREAD_H
