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

#ifndef KAIDAN_H
#define KAIDAN_H

// Qt
#include <QObject>
#include <QString>
// gloox
#include <gloox/connectionlistener.h>
// Kaidan
#include "ClientThread.h"
#include "Globals.h"
#include "Enums.h"

class AvatarFileStorage;
class RosterManager;
class RosterModel;
class MessageModel;
class QSettings;
class Database;

using namespace Enums;

/**
 * @class Kaidan Kaidan's Back-End Class
 * 
 * @brief This class will initiate the complete back-end, including the @see Database
 * connection, viewing models (@see MessageModel, @see RosterModel), etc.
 * 
 * This class will run in the main thread, only the XMPP connection runs in another
 * thread (@see ClientThread).
 */
class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterModel* rosterModel READ getRosterModel NOTIFY rosterModelChanged)
	Q_PROPERTY(MessageModel* messageModel READ getMessageModel NOTIFY messageModelChanged)
	Q_PROPERTY(AvatarFileStorage* avatarStorage READ getAvatarStorage NOTIFY avatarStorageChanged)
	Q_PROPERTY(quint8 connectionState READ getConnectionState NOTIFY connectionStateChanged)
	Q_PROPERTY(quint8 disconnReason READ getDisconnReason NOTIFY disconnReasonChanged)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString jidResource READ getJidResource WRITE setJidResource NOTIFY jidResourceChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged)
	Q_PROPERTY(QString chatPartner READ getChatPartner WRITE setChatPartner NOTIFY chatPartnerChanged)

public:
	/**
	 * Constructor
	 */
	Kaidan(QObject *parent = 0);

	/**
	 * Destructor
	 */
	~Kaidan();

	/**
	 * Start connection (called from QML when ready)
	 */
	Q_INVOKABLE void start();

	/**
	 * Connect to the XMPP server
	 *
	 * If you haven't set a username and password, they are used from the
	 * last successful login (the settings file).
	 */
	Q_INVOKABLE bool mainConnect();

	/**
	 * Disconnect from XMPP server
	 *
	 * This will disconnect the client from the server. When disconnected,
	 * the connectionStateChanged signal will be emitted.
	 * 
	 * @param openLogInPage If true, the newCredentialsNeeded signal will be
	 * emitted.
	 */
	Q_INVOKABLE void mainDisconnect(bool openLogInPage = false);

	/**
	 * Send a text message to any JID
	 *
	 * Currently only contacts are displayed on the RosterPage (there is no
	 * way to view a list of all chats -> for contacts and non-contacts), so
	 * you should only send messages to JIDs from your roster, otherwise you
	 * won't be able to see the message history.
	 */
	Q_INVOKABLE void sendMessage(QString jid, QString message);

	/**
	 * Add a contact to your roster
	 *
	 * @param nick A simple nick name for the new contact, which should be
	 *             used to display in the roster.
	 */
	Q_INVOKABLE void addContact(QString jid, QString nick);

	/**
	 * Remove a contact from your roster
	 *
	 * Only the JID is needed.
	 */
	Q_INVOKABLE void removeContact(QString jid);

	/**
	 * Returns a URL to a given resource file name
	 *
	 * This will check various paths which could contain the searched file.
	 * If the file was found, it'll return a `file://` or a `qrc:/` url to
	 * the file.
	 */
	Q_INVOKABLE QString getResourcePath(QString resourceName) const;

	/**
	 * Returns the current ConnectionState
	 */
	Q_INVOKABLE quint8 getConnectionState() const;

	/**
	 * Returns the last disconnection reason
	 */
	Q_INVOKABLE quint8 getDisconnReason() const;

	/**
	 * Returns a string of this build's Kaidan version
	 */
	Q_INVOKABLE QString getVersionString() const
	{
		return QString(VERSION_STRING);
	}

	/**
	 * Returns a string without new lines, unneeded spaces, etc.
	 *
	 * See QString::simplified for more information.
	 */
	Q_INVOKABLE QString removeNewLinesFromString(QString input) const
	{
		return input.simplified();
	}

	/**
	 * Set own JID used for connection
	 *
	 * To really change the JID of the current connection, you'll need to
	 * reconnect.
	 */
	void setJid(QString jid);

	/**
	 * Get the current JID
	 */
	QString getJid() const
	{
		return creds.jid;
	}

	/**
	 * Set a optional custom JID resource (device name)
	 */
	void setJidResource(QString jidResource);

	/**
	 * Get the JID resoruce
	 */
	QString getJidResource() const
	{
		return creds.jidResource;
	}

	/**
	 * Set the password for next connection
	 */
	void setPassword(QString password);

	/**
	 * Get the currently used password
	 */
	QString getPassword() const
	{
		return creds.password;
	}

	/**
	 * Set the currently opened chat
	 *
	 * This will set a filter on the database to only view the wanted messages.
	 */
	void setChatPartner(QString jid);

	/**
	 * Get the currrently opened chat
	 */
	QString getChatPartner() const
	{
		return chatPartner;
	}

	/**
	 * Get the roster model
	 */
	RosterModel* getRosterModel() const
	{
		return rosterModel;
	}

	/**
	 * Get the message model
	 */
	MessageModel* getMessageModel() const
	{
		return messageModel;
	}

	/**
	 * Get the avatar storage
	 */
	AvatarFileStorage* getAvatarStorage() const
	{
		return avatarStorage;
	}

signals:
	void rosterModelChanged();
	void messageModelChanged();
	void avatarStorageChanged();

	/**
	 * Emitted, when the client's connection state has changed (e.g. when
	 * successfully connected or when disconnected)
	 */
	void connectionStateChanged(quint8 state);

	/**
	 * Emitted, when the client failed to connect and gives the reason in
	 * a DisconnectionReason enumatrion.
	 */
	void disconnReasonChanged(quint8 reason);

	/**
	 * Emitted when the JID was changed
	 */
	void jidChanged();

	/**
	 * Emitted when the JID resouce (device name) has changed
	 */
	void jidResourceChanged();

	/**
	 * Emitted when the used password for logging in has changed
	 */
	void passwordChanged();

	/**
	 * Emitted when the currently opnened chat has changed
	 */
	void chatPartnerChanged();

	/**
	 * Emitted when there are no (correct) credentials and new are needed
	 *
	 * The client will be in disconnected state, when this is emitted.
	 */
	void newCredentialsNeeded();

	/**
	 * Emitted when log in worked with new credentials
	 *
	 * The client will be in connected state, when this is emitted.
	 */
	void logInWorked();

private:
	void connectDatabases();

	ClientThread *client;
	Database *database;
	RosterModel *rosterModel;
	RosterManager *rosterManager;
	MessageModel *messageModel;
	AvatarFileStorage *avatarStorage;
	QSettings *settings;

	ClientThread::Credentials creds;
	QString chatPartner;
};

#endif
