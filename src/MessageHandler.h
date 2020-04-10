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

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

// Qt
#include <QObject>
// QXmpp
#include <QXmppGlobal.h>
#include <QXmppMessageReceiptManager.h>
// Kaidan
#include "Message.h"

class ClientWorker;
class Kaidan;
class MessageModel;
class QXmppMessage;
class QXmppDiscoveryIq;
class QXmppCarbonManager;

/**
 * @class MessageHandler Handler for incoming and outgoing messages.
 */
class MessageHandler : public QObject
{
	Q_OBJECT

public:
	MessageHandler(Kaidan *kaidan, ClientWorker *clientWorker, QXmppClient *client, MessageModel *model);

	~MessageHandler();

public slots:
	/**
	 * Handles incoming messages from the server.
	 */
	void handleMessage(const QXmppMessage &msg);

	/**
	 * Sends a new message to the server and inserts it into the database.
	 */
	void sendMessage(const QString& toJid, const QString& body, bool isSpoiler, const QString& spoilerHint);

	/**
	 * Sends the corrected version of a message.
	 */
	void correctMessage(const QString& toJid, const QString& msgId, const QString& newBody);

	/**
	 * Handles service discovery info and enables carbons if feature was found.
	 */
	void handleDiscoInfo(const QXmppDiscoveryIq &);

private slots:
	/**
	 * Handles pending messages found in the database.
	 */
	void handlePendingMessages(const QVector<Message> &messages);

	void sendPendingMessage(const Message& message);

private:
	Kaidan *kaidan;
	ClientWorker *m_clientWorker;
	QXmppClient *client;
	QXmppMessageReceiptManager receiptManager;
	MessageModel *model;
	QXmppCarbonManager *carbonManager;
};

#endif // MESSAGEHANDLER_H
