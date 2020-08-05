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

#include "PresenceCache.h"
#include "Enums.h"
#include <QXmppUtils.h>

using namespace Enums;

PresenceCache::PresenceCache(QObject *parent)
    : QObject(parent)
{
}

void PresenceCache::updatePresence(const QXmppPresence &presence)
{
	QString jid = QXmppUtils::jidToBareJid(presence.from());
	QString resource = QXmppUtils::jidToResource(presence.from());

	if (!m_presences.contains(jid))
		m_presences.insert(jid, {});

	m_presences[jid][resource] = presence;

	emit presenceChanged(jid);
}

void PresenceCache::clear()
{
	m_presences.clear();
}

quint8 PresenceCache::getPresenceType(const QString &bareJid)
{
	if (!m_presences.contains(bareJid))
		return quint8(AvailabilityTypes::PresUnavailable);

	const auto pres = m_presences.value(bareJid).last();

	if (pres.type() == QXmppPresence::Unavailable) {
		return quint8(AvailabilityTypes::PresUnavailable);
	} else if (pres.type() == QXmppPresence::Available) {
		switch (pres.availableStatusType()) {
		case QXmppPresence::Online:
			return quint8(AvailabilityTypes::PresOnline);
		case QXmppPresence::Away:
			return quint8(AvailabilityTypes::PresAway);
		case QXmppPresence::XA:
			return quint8(AvailabilityTypes::PresXA);
		case QXmppPresence::DND:
			return quint8(AvailabilityTypes::PresDND);
		case QXmppPresence::Chat:
			return quint8(AvailabilityTypes::PresChat);
		case QXmppPresence::Invisible:
			return quint8(AvailabilityTypes::PresInvisible);
		default:
			return quint8(AvailabilityTypes::PresUnavailable);
		}
	} else if (pres.type() == QXmppPresence::Error) {
		return quint8(AvailabilityTypes::PresError);
	}
	return quint8(AvailabilityTypes::PresUnavailable);
}

QString PresenceCache::getStatusText(const QString &bareJid)
{
	if (!m_presences.contains(bareJid))
		return {};

	return m_presences[bareJid].last().statusText();
}
