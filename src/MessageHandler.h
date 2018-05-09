/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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
#include <QSqlDatabase>
// gloox
#include <gloox/messagehandler.h>
// Kaidan
#include "MessageModel.h"
#include "RosterModel.h"

namespace gloox {
	class Client;
	class Message;
	class MessageSession;
}

class MessageHandler : public QObject, public gloox::MessageHandler
{
	Q_OBJECT

public:
	MessageHandler(gloox::Client *client, MessageModel *messageModel,
		       RosterModel *rosterModel, QObject *parent = nullptr);
	~MessageHandler();

	virtual void handleMessage(const gloox::Message &message, gloox::MessageSession *session = 0);

	/**
	 * Handles a message with a possible receipt or receipt request
	 */
	void handleReceiptMessage(const gloox::Message *message,
	                          bool isCarbonMessage);

	void updateLastExchangedOfJid(const QString &jid);
	void newUnreadMessageForJid(const QString &jid);
	void resetUnreadMessagesForJid(const QString &jid);

public slots:
	void setChatPartner(QString chatPartner);
	void sendMessage(QString toJid, QString body);

private:
	gloox::Client *client;
	MessageModel *messageModel;
	RosterModel *rosterModel;
	QString chatPartner;
};

#endif // MESSAGEHANDLER_H
