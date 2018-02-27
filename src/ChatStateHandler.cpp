/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2018 Kaidan developers and contributors
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

#include "ChatStateHandler.h"
// Kaidan
#include "ChatStateCache.h"
// Qt
#include <QDebug>
// gloox
#include <gloox/jid.h>
#include <gloox/chatstatefilter.h>

ChatStateHandler::ChatStateHandler(ChatStateCache *cache, QObject *parent)
	: QObject(parent), cache(cache)
{
	filters = QMap<gloox::JID, gloox::ChatStateFilter*>();
	chatStates = QMap<QString, ChatStateType>();
}

ChatStateHandler::~ChatStateHandler()
{
	for (gloox::JID &jid : filters.keys())
		removeChatStateFilter(jid);
}

void ChatStateHandler::handleChatState(const gloox::JID &from, gloox::ChatStateType state)
{
	// update chat state of chat partner
	cache->setState(QString::fromStdString(from.bare()), (ChatStateType) state);

	qDebug() << "[client] handleChatState:" << QString::fromStdString(from.full()) << state;
}

void ChatStateHandler::registerMessageSession(gloox::MessageSession* session)
{
	if (filters.contains(session->target().bareJID()))
		removeChatStateFilter(session->target().bareJID());

	gloox::ChatStateFilter *filter = new gloox::ChatStateFilter(session);
	filters[session->target()] = filter;

	filter->registerChatStateHandler(this);
}

void ChatStateHandler::removeChatStateFilter(const gloox::JID &jid)
{
	delete filters[jid];
	filters.remove(jid);
}

void ChatStateHandler::handleConnectionState(ConnectionState state)
{
	if (state == ConnectionState::StateDisconnected) {
		for (QString &jid : chatStates.keys()) {
			chatStates[jid] = ChatStateType::ChatStateInvalid;
		}

		// delete all exsisting message session filters
		for (gloox::JID &jid : filters.keys())
			removeChatStateFilter(jid);
	}
}

void ChatStateHandler::handleMessageTyped()
{
	qDebug() << QString("[client] Changing own chat state with %1 to 'typing'")
	            .arg("DEINERMUTTER");
	
}
