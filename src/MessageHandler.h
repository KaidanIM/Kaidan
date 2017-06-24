/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

// Qt
#include <QObject>
#include <QSqlDatabase>
// gloox
#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/messagehandler.h>
#include <gloox/messagesession.h>
// Kaidan
#include "MessageModel.h"

class MessageHandler : public gloox::MessageHandler
{
public:
	MessageHandler(gloox::Client *client, MessageModel *model);
	~MessageHandler();

	void sendMessage(QString *fromJid, QString *toJid, QString *body);
	virtual void handleMessage(const gloox::Message &message, gloox::MessageSession *session = 0);

private:
	gloox::Client *client;
	MessageModel *messageModel;
};

#endif // MESSAGEHANDLER_H
