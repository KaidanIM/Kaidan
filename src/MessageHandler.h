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
#include <QObject>
// QXmpp
#include <QXmppGlobal.h>
#include <QXmppMamManager.h>
#include <QXmppMessageReceiptManager.h>
// Kaidan
#include "Message.h"
#include "Enums.h"

class ClientWorker;
class Kaidan;

class QMimeType;

class QXmppMessage;
class QXmppDiscoveryIq;
class QXmppCarbonManager;
class QXmppVersionIq;

/**
 * @class MessageHandler Handler for incoming and outgoing messages.
 */
class MessageHandler : public QObject
{
	Q_OBJECT

public:
	MessageHandler(ClientWorker *clientWorker, QXmppClient *client, QObject *parent = nullptr);
	~MessageHandler();

public slots:
	void handleRosterReceived();
	void handleLastMessageStampFetched(const QDateTime &stamp);

	/**
	 * Handles incoming messages from the server.
	 */
	void handleMessage(const QXmppMessage &msg, MessageOrigin origin);

	/**
	 * Send a text message to any JID
	 */
	void sendMessage(const QString &toJid, const QString &body, bool isSpoiler, const QString &spoilerHint);

	/**
	 * Sends a chat state notification to the server.
	 */
	void sendChatState(const QString &toJid, const QXmppMessage::State state);

	/**
	 * Sends the corrected version of a message.
	 */
	void sendCorrectedMessage(const Message &msg);

	/**
	 * Handles service discovery info and enables carbons if feature was found.
	 */
	void handleDiscoInfo(const QXmppDiscoveryIq &);

signals:
	void sendMessageRequested(const QString &toJid,
				  const QString &body,
				  bool isSpoiler,
				  const QString &spoilerHint);

	void retrieveBacklogMessagesRequested(const QString &jid, const QDateTime &stamp);

private slots:
	void handleConnected();
	void handleDisonnected();

	/**
	 * Handles pending messages found in the database.
	 */
	void handlePendingMessages(const QVector<Message> &messages);

	void sendPendingMessage(const Message &message);

	void handleArchiveMessage(const QString &queryId, const QXmppMessage &message);
	void handleArchiveResults(const QString &queryId,
	                          const QXmppResultSetReply &resultSetReply,
	                          bool complete);

	void retrieveInitialMessages();
	void retrieveCatchUpMessages(const QDateTime &stamp);
	void retrieveBacklogMessages(const QString &jid, const QDateTime &last);

private:
	bool parseMediaUri(Message &message, const QString &uri, bool isBodyPart);

	struct BacklogQueryState {
		QString chatJid;
		QDateTime lastTimestamp;
	};

	ClientWorker *m_clientWorker;
	QXmppClient *m_client;
	QXmppMessageReceiptManager m_receiptManager;
	QXmppCarbonManager *m_carbonManager;
	QXmppMamManager *m_mamManager;

	QDateTime m_lastMessageStamp;
	bool m_lastMessageLoaded = false;

	// All messages after the intitial message
	QVector<QString> m_runningInitialMessageQueryIds;
	// Mapping of all running MAM backlog queries to their chat JIDs
	QMap<QString, BacklogQueryState> m_runningBacklogQueryIds;
	// query id of the MAM query for catching up all missing messages
	QString m_runnningCatchUpQueryId;
};
