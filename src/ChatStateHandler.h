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

#ifndef CHATSTATEHANDLER_H
#define CHATSTATEHANDLER_H

// Kaidan
#include "Enums.h"
class ChatStateCache;
// gloox
#include <gloox/chatstatehandler.h>
namespace gloox {
	class MessageSession;
	class ChatStateFilter;
	class JID;
}
// Qt
#include <QObject>
#include <QMap>
class QTimer;

using namespace Enums;

/**
 * @class ChatStateHandler Class for handling typing and online notifications
 */
class ChatStateHandler : public QObject, public gloox::ChatStateHandler
{
	Q_OBJECT

public:
	ChatStateHandler(ChatStateCache *cache, QObject *parent = nullptr);
	~ChatStateHandler();

	/**
	 * Handles a chat state and will update the UI
	 *
	 * @param from JID which sent the chat state notification
	 * @param session The current message session with a JID
	 */
	virtual void handleChatState(const gloox::JID &from, gloox::ChatStateType state);

	/**
	 * Register a new message session for receiving chat state notifications
	 */
	void registerMessageSession(gloox::MessageSession *session);

	/**
	 * Removes and deletes the chat state filter of this JID
	 */
	void removeChatStateFilter(const gloox::JID &jid);

	/**
	 * Sets chat states to inactive, when disconnected
	 */
	void handleConnectionState(ConnectionState state);

	/**
	 * Updates the current chat partner and will trigger the
	 */
	void setChatPartner(QString &chatPartner);

public slots:
	/**
	 * Sets the current chat state with the current chat partner to 'typing'
	 *
	 * It'll also start a timer that will turn the state to 'paused', when
	 * the signal isn't emitted again for some time.
	 */
	void handleMessageTyped();

private:
	ChatStateCache *cache;
	QMap<gloox::JID, gloox::ChatStateFilter*> filters;
	QMap<QString, ChatStateType> chatStates;
	QTimer *timer;

	QString chatPartner;
};

#endif // CHATSTATEHANDLER_H
