/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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
#include <QColor>
#include <QObject>
#include <QString>
// Kaidan
#include "ClientWorker.h"
#include "Enums.h"
#include "Globals.h"

class QGuiApplication;
class Database;
class QXmppClient;
class QmlUtils;

using namespace Enums;

/**
 * @class Kaidan Kaidan's Back-End Class
 *
 * @brief This class will initiate the complete back-end, including the @see Database
 * connection, viewing models (@see MessageModel, @see RosterModel), etc.
 *
 * This class will run in the main thread, the XMPP connection and the database managers
 * run in other threads.
 */
class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QmlUtils* utils READ getUtils CONSTANT)
	Q_PROPERTY(RosterModel* rosterModel READ getRosterModel CONSTANT)
	Q_PROPERTY(MessageModel* messageModel READ getMessageModel CONSTANT)
	Q_PROPERTY(AvatarFileStorage* avatarStorage READ getAvatarStorage NOTIFY avatarStorageChanged)
	Q_PROPERTY(PresenceCache* presenceCache READ getPresenceCache CONSTANT)
	Q_PROPERTY(TransferCache* transferCache READ getTransferCache CONSTANT)
	Q_PROPERTY(QSettings* settings READ getSettings CONSTANT)
	Q_PROPERTY(quint8 connectionState READ getConnectionState NOTIFY connectionStateChanged)
	Q_PROPERTY(quint8 disconnReason READ getDisconnReason NOTIFY disconnReasonChanged)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString jidResource READ getJidResource WRITE setJidResource NOTIFY jidResourceChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged)
	Q_PROPERTY(bool uploadServiceFound READ getUploadServiceFound NOTIFY uploadServiceFoundChanged)

public:
	static Kaidan *instance();

	Kaidan(QGuiApplication *app, bool enableLogging = true, QObject *parent = nullptr);
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
	Q_INVOKABLE void mainConnect();

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
	 * Returns the current ConnectionState
	 */
	Q_INVOKABLE quint8 getConnectionState() const
	{
		return (quint8) connectionState;
	}

	/**
	 * Returns the last disconnection reason
	 */
	Q_INVOKABLE quint8 getDisconnReason() const;

	/**
	 * Set own JID used for connection
	 *
	 * To really change the JID of the current connection, you'll need to
	 * reconnect.
	 */
	void setJid(const QString &jid);

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
	void setJidResource(const QString &jidResource);

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
	void setPassword(const QString &password);

	/**
	 * Get the currently used password
	 */
	QString getPassword() const
	{
		return creds.password;
	}

	RosterModel* getRosterModel() const
	{
		return m_caches->rosterModel;
	}

	MessageModel* getMessageModel() const
	{
		return m_caches->msgModel;
	}

	AvatarFileStorage* getAvatarStorage() const
	{
		return m_caches->avatarStorage;
	}

	PresenceCache* getPresenceCache() const
	{
		return m_caches->presCache;
	}

	TransferCache* getTransferCache() const
	{
		return m_caches->transferCache;
	}

	QSettings* getSettings() const
	{
		return m_caches->settings;
	}

	QmlUtils* getUtils() const
	{
		return m_utils;
	}

	ClientWorker *getClient() const;

	/**
	 * Adds XMPP URI to open as soon as possible
	 */
	void addOpenUri(const QString &uri);

    /**
     * Connects to the server by the parsed credentials (bare JID and password)
     * from a given XMPP URI (e.g. from scanning a QR code)
     * like "xmpp:user@example.org?login;password=abc"
     */
    Q_INVOKABLE void loginByUri(const QString &uri);

	/**
	 * Returns whether an HTTP File Upload service has been found
	 */
	bool getUploadServiceFound() const
	{
		return uploadServiceFound;
	}

signals:
	void avatarStorageChanged();

	/**
	 * Emitted, when the client's connection state has changed (e.g. when
	 * successfully connected or when disconnected)
	 */
	void connectionStateChanged();

	/**
	 * Emitted, when the client failed to connect and gives the reason in
	 * a DisconnectionReason enumatrion.
	 */
	void disconnReasonChanged();

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

	/**
	 * Show passive notification
	 */
	void passiveNotificationRequested(QString text);

	/**
	 * Emitted, whan a subscription request was received
	 */
	void subscriptionRequestReceived(QString from, QString msg);

	/**
	 * Incoming subscription request was accepted or declined by the user
	 */
	void subscriptionRequestAnswered(QString jid, bool accepted);

	/**
	 * Request vCard of any JID
	 *
	 * Is required when the avatar (or other information) of a JID are
	 * requested and the JID is not in the roster.
	 */
	void vCardRequested(const QString &jid);

	/**
	 * XMPP URI received
	 *
	 * Is called when Kaidan was used to open an XMPP URI (i.e. 'xmpp:kaidan@muc.kaidan.im?join')
	 */
	void xmppUriReceived(QString uri);

	/**
	 * The upload progress of a file upload has changed
	 */
	void uploadProgressMade(QString msgId, quint64 sent, quint64 total);

	/**
	 * An HTTP File Upload service was discovered
	 */
	void uploadServiceFoundChanged();

	/**
	 * Send a text message to any JID
	 *
	 * Currently only contacts are displayed on the RosterPage (there is no
	 * way to view a list of all chats -> for contacts and non-contacts), so
	 * you should only send messages to JIDs from your roster, otherwise you
	 * won't be able to see the message history.
	 */
	void sendMessage(QString jid, QString message, bool isSpoiler, QString spoilerHint);

	/**
	 * Correct the last message
	 *
	 * To get/check the last message id, use `kaidan.messageModel.lastMessageId(jid)`
	 */
	void correctMessage(QString toJid, QString msgId, QString message);

	/**
	 * Upload and send file
	 */
	void sendFile(QString jid, QString filePath, QString message);

	/**
	 * Add a contact to your roster
	 *
	 * @param nick A simple nick name for the new contact, which should be
	 *             used to display in the roster.
	 */
	void addContact(QString jid, QString nick, QString msg);

	/**
	 * Remove a contact from your roster
	 *
	 * Only the JID is needed.
	 */
	void removeContact(QString jid);

	/**
	 * Change a contact's name
	 */
	void renameContact(const QString &jid, const QString &newContactName);

	/**
	 * Downloads an attached media file of a message
	 *
	 * @param msgId The message
	 * @param url the media url from the message
	 */
	void downloadMedia(QString msgId, QString url);

	/**
	 * Changes the user's password on the server
	 *
	 * @param newPassword The new password
	 */
	void changePassword(const QString &newPassword);

	/**
	 * Emitted, when changing the password has succeeded.
	 */
	void passwordChangeSucceeded();

	/**
	 * Emitted, when changing the password has failed.
	 */
	void passwordChangeFailed();

	/**
	 * Emitted, when a contact was muted/unmuted.
	 */
	void notificationsMutedChanged(const QString& jid);

public slots:
	/**
	 * Set current connection state
	 */
	void setConnectionState(QXmppClient::State state);

	/**
	 * Sets the disconnection error/reason
	 */
	void setDisconnReason(Enums::DisconnectionReason reason);

	/**
	 * Receives messages from another instance of the application
	 */
	void receiveMessage(quint32, const QByteArray &msg)
	{
		// currently we only send XMPP URIs
		addOpenUri(msg);
	}

	/**
	 * Enables HTTP File Upload to be used (will be called from UploadManager)
	 */
	void setUploadServiceFound(bool enabled)
	{
		uploadServiceFound = enabled;
		emit uploadServiceFoundChanged();
	}

	/**
	 * Returns whether notifications are enabled for the given contact.
	 */
	bool notificationsMuted(const QString& jid);

	/**
	 * Sets the notifications to muted/unmuted.
	 *
	 * @param muted true if notifications should be muted.
	 * @param jid contains the current chatpartner's jid.
	 */
	void setNotificationsMuted(const QString& jid, bool muted);

private:
	void connectDatabases();

    /**
     * Notifies if no login URI was found
     */
    void notifyLoginUriNotFound();

	QmlUtils *m_utils;
	Database *m_database;
	QThread *m_dbThrd;
	MessageDb *m_msgDb;
	RosterDb *m_rosterDb;
	QThread *m_cltThrd;
	ClientWorker::Caches *m_caches;
	ClientWorker *m_client;

	ClientWorker::Credentials creds;
	QString openUriCache;
	bool uploadServiceFound = false;
	ConnectionState connectionState = ConnectionState::StateDisconnected;
	DisconnReason disconnReason = DisconnReason::ConnNoError;

	static Kaidan *s_instance;
};

#endif
