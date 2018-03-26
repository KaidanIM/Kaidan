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

#include "MessageSessionHandler.h"
#include "MessageHandler.h"
#include "ChatStateHandler.h"
// gloox
#include <gloox/chatstatefilter.h>

MessageSessionHandler::MessageSessionHandler(gloox::Client *client,
                                             MessageModel *messageModel,
                                             RosterModel *rosterModel,
                                             ChatStateCache *chatStateCache,
                                             QObject *parent)
	: QObject(parent), client(client)
{
	client->registerMessageSessionHandler(this);
	messageHandler = new MessageHandler(client, messageModel, rosterModel);
	chatStateHandler = new ChatStateHandler(chatStateCache);

	msgSessions = QList<gloox::MessageSession*>();
}

MessageSessionHandler::~MessageSessionHandler()
{
	delete messageHandler;
	delete chatStateHandler;
}

MessageHandler* MessageSessionHandler::getMessageHandler()
{
	return messageHandler;
}

ChatStateHandler* MessageSessionHandler::getChatStateHandler()
{
	return chatStateHandler;
}

void MessageSessionHandler::handleMessageSession(gloox::MessageSession *session)
{
	// close old message sessions with this JID
	disposeMessageSessions(session->target().bareJID());
	// save new session
	msgSessions << session;

	session->registerMessageHandler(messageHandler);
	chatStateHandler->registerMessageSession(session);
}

void MessageSessionHandler::disposeMessageSessions(const gloox::JID &jid)
{
	for (gloox::MessageSession *session : msgSessions) {
		if (session->target().bareJID() == jid.bareJID()) {
			msgSessions.removeOne(session);
			client->disposeMessageSession(session);
		}
	}
}

void MessageSessionHandler::handleConnectionState(ConnectionState state)
{
	chatStateHandler->handleConnectionState(state);

	if (state == ConnectionState::StateDisconnected) {
		for (gloox::MessageSession *session : msgSessions) {
			msgSessions.removeOne(session);
			client->disposeMessageSession(session);
		}
	}
}

void MessageSessionHandler::handleChatPartner(QString chatPartner)
{
	this->chatPartner = chatPartner;
	chatStateHandler->setChatPartner(chatPartner);
}
