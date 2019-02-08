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

#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QDateTime>
#include "QXmppRosterIq.h"

/**
 * Item containing one contact / conversation.
 */
class RosterItem
{
public:
	RosterItem() = default;
	RosterItem(const QXmppRosterIq::Item &item);

	QString jid() const;
	void setJid(const QString &jid);

	QString name() const;
	void setName(const QString &name);

	int unreadMessages() const;
	void setUnreadMessages(int unreadMessages);

	QDateTime lastExchanged() const;
	void setLastExchanged(const QDateTime &lastExchanged);

	QString lastMessage() const;
	void setLastMessage(const QString &lastMessage);

	bool operator==(const RosterItem &other) const;
	bool operator!=(const RosterItem &other) const;

private:
	/**
	 * JID of the contact.
	 */
	QString m_jid;

	/**
	 * Name of the contact.
	 */
	QString m_name;

	/**
	 * Number of messages unread by the user.
	 */
	int m_unreadMessages = 0;

	/**
	 * Last activity of the conversation, e.g. an incoming message.
	 * This is used to sort the contacts on the roster page.
	 */
	QDateTime m_lastExchanged = QDateTime::currentDateTimeUtc();

	/**
	 * Last message of the conversation.
	 */
	QString m_lastMessage;
};

#endif // ROSTERITEM_H
