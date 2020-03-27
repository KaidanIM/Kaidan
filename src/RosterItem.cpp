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

#include "RosterItem.h"

RosterItem::RosterItem(const QXmppRosterIq::Item &item, const QDateTime &dateTime)
	: m_jid(item.bareJid()), m_name(item.name()), m_lastExchanged(dateTime)
{
}

QString RosterItem::jid() const
{
	return m_jid;
}

void RosterItem::setJid(const QString &jid)
{
	m_jid = jid;
}

QString RosterItem::name() const
{
	return m_name;
}

void RosterItem::setName(const QString &name)
{
	m_name = name;
}

int RosterItem::unreadMessages() const
{
	return m_unreadMessages;
}

void RosterItem::setUnreadMessages(int unreadMessages)
{
	m_unreadMessages = unreadMessages;
}

QDateTime RosterItem::lastExchanged() const
{
	return m_lastExchanged;
}

void RosterItem::setLastExchanged(const QDateTime &lastExchanged)
{
	m_lastExchanged = lastExchanged;
}

QString RosterItem::lastMessage() const
{
	return m_lastMessage;
}

void RosterItem::setLastMessage(const QString &lastMessage)
{
	m_lastMessage = lastMessage;
}

QString RosterItem::displayName() const
{
	return m_name.isEmpty() ? m_jid : m_name;
}

bool RosterItem::operator==(const RosterItem &other) const
{
	return m_jid == other.jid() &&
	       m_name == other.name() &&
	       m_lastMessage == other.lastMessage() &&
	       m_lastExchanged == other.lastExchanged() &&
	       m_unreadMessages == other.unreadMessages();
}

bool RosterItem::operator!=(const RosterItem &other) const
{
	return !operator==(other);
}

bool RosterItem::operator<(const RosterItem &other) const
{
	if (lastExchanged() != other.lastExchanged())
		return lastExchanged() > other.lastExchanged();
	return displayName().toUpper() < other.displayName().toUpper();
}

bool RosterItem::operator>(const RosterItem &other) const
{
	if (lastExchanged() != other.lastExchanged())
		return lastExchanged() < other.lastExchanged();
	return displayName().toUpper() > other.displayName().toUpper();
}

bool RosterItem::operator<=(const RosterItem &other) const
{
	if (lastExchanged() != other.lastExchanged())
		return lastExchanged() >= other.lastExchanged();
	return displayName().toUpper() <= other.displayName().toUpper();
}

bool RosterItem::operator>=(const RosterItem &other) const
{
	if (lastExchanged() != other.lastExchanged())
		return lastExchanged() <= other.lastExchanged();
	return displayName().toUpper() >= other.displayName().toUpper();
}
