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

#include "PresenceCache.h"
// Qt
#include <QColor>
// QXmpp
#include <QXmppUtils.h>

PresenceCache *PresenceCache::s_instance = nullptr;

QString Presence::availabilityToIcon(Availability type)
{
	switch (type) {
	case Online:
		return QStringLiteral("im-user-online");
	case Chat:
		return QStringLiteral("im-user-online");
	case Away:
		return QStringLiteral("im-user-away");
	case DND:
		return QStringLiteral("im-kick-user");
	case XA:
		return QStringLiteral("im-user-away");
	case Offline:
		return QStringLiteral("im-user-offline");
	}
	Q_UNREACHABLE();
	return {};
}

QString Presence::availabilityToText(Availability type)
{
	switch (type) {
	case Online:
		return QObject::tr("Available");
	case Chat:
		return QObject::tr("Free for chat");
	case Away:
		return QObject::tr("Away");
	case DND:
		return QObject::tr("Do not disturb");
	case XA:
		return QObject::tr("Away for longer");
	case Offline:
		return QObject::tr("Offline");
	}
	Q_UNREACHABLE();
	return {};
}

QColor Presence::availabilityToColor(Availability type)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	switch (type) {
	case Online:
		return QColorConstants::Svg::green;
	case Chat:
		return QColorConstants::Svg::darkgreen;
	case Away:
		return QColorConstants::Svg::orange;
	case DND:
		return QColor::fromRgb(218, 68, 83);
	case XA:
		return QColorConstants::Svg::orange;
	case Offline:
		return QColorConstants::Svg::silver;
	}
#else
	switch (type) {
	case Online:
		return "green";
	case Chat:
		return "darkgreen";
	case Away:
		return "orange";
	case DND:
		return QColor::fromRgb(218, 68, 83);
	case XA:
		return "orange";
	case Offline:
		return "silver";
	}
#endif
	Q_UNREACHABLE();
	return {};
}

PresenceCache::PresenceCache(QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(!s_instance);
	s_instance = this;
}

PresenceCache::~PresenceCache()
{
	s_instance = nullptr;
}

QString PresenceCache::pickIdealResource(const QString &jid)
{
	if (!m_presences.contains(jid))
		return {};

	const auto &userPresences = m_presences[jid];
	if (userPresences.isEmpty())
		return {};
	if (userPresences.size() == 1)
		return userPresences.firstKey();

	auto result = userPresences.cbegin();
	for (auto itr = result + 1; itr != userPresences.cend(); itr++) {
		if (presenceMoreImportant(*itr, *result))
			result = itr;
	}

	return result.key();
}

std::optional<QXmppPresence> PresenceCache::presence(const QString &jid, const QString &resource)
{
	if (const auto itr = m_presences.constFind(jid); itr != m_presences.cend()) {
		if (const auto resourceItr = itr->constFind(resource); resourceItr != itr->cend()) {
			return *resourceItr;
		}
	}
	return std::nullopt;
}

void PresenceCache::updatePresence(const QXmppPresence &presence)
{
	if (presence.type() != QXmppPresence::Available && presence.type() != QXmppPresence::Unavailable)
		return;

	const auto jid = QXmppUtils::jidToBareJid(presence.from());
	const auto resource = QXmppUtils::jidToResource(presence.from());

	if (!m_presences.contains(jid))
		m_presences.insert(jid, {});

	auto &userPresences = m_presences[jid];

	//
	// Presence updates can only go this way:
	//                  /---------------------------v
	// +-----------+   /   +---------+       +--------------+
	// | Connected | ----> | Updated | ----> | Disconnected |
	// +-----------+       +---------+  \    +--------------+
	//                          ^_______/
	//

	if (userPresences.contains(resource)) {
		if (presence.type() == QXmppPresence::Available) {
			m_presences[jid][resource] = presence;
			emit presenceChanged(Updated, jid, resource);
		} else {
			// presence is 'Unavailable'
			userPresences.remove(resource);
			if (userPresences.isEmpty())
				m_presences.remove(jid);

			emit presenceChanged(Disconnected, jid, resource);
		}
	} else {
		// client is unknown (hasn't been cached yet)
		if (presence.type() == QXmppPresence::Available) {
			userPresences.insert(resource, presence);
			emit presenceChanged(Connected, jid, resource);
		}

		// presences from unknown clients that are unavailable are ignored
	}
}

void PresenceCache::clear()
{
	m_presences.clear();
	emit presencesCleared();
}

constexpr qint8 PresenceCache::availabilityPriority(QXmppPresence::AvailableStatusType type)
{
	switch (type) {
	case QXmppPresence::XA:
	case QXmppPresence::Away:
		return 0;
	case QXmppPresence::Online:
		return 1;
	case QXmppPresence::Chat:
		return 2;
	case QXmppPresence::DND:
		return 3;
	default:
		return -1;
	}
}

bool PresenceCache::presenceMoreImportant(const QXmppPresence &a, const QXmppPresence &b)
{
	if (a.priority() != b.priority())
		return a.priority() > b.priority();

	if (const auto aAvailable = availabilityPriority(a.availableStatusType()),
		bAvailable = availabilityPriority(b.availableStatusType());
		aAvailable != bAvailable) {
		return aAvailable > bAvailable;
	}

	return !a.statusText().isEmpty() > !b.statusText().isEmpty();
}

UserPresenceWatcher::UserPresenceWatcher(QObject *parent)
	: QObject(parent), m_resourceAutoPicked(true)
{
	connect(PresenceCache::instance(), &PresenceCache::presenceChanged, this, &UserPresenceWatcher::handlePresenceChanged);
	connect(PresenceCache::instance(), &PresenceCache::presencesCleared, this, &UserPresenceWatcher::handlePresencesCleared);
}

Presence::Availability UserPresenceWatcher::availability() const
{
	if (const auto presence = PresenceCache::instance()->presence(m_jid, m_resource)) {
		return Presence::availabilityFromAvailabilityStatusType(
			presence->availableStatusType());
	}
	return Presence::Offline;
}

QString UserPresenceWatcher::availabilityIcon() const
{
	return Presence::availabilityToIcon(availability());
}

QString UserPresenceWatcher::availabilityText() const
{
	return Presence::availabilityToText(availability());
}

QColor UserPresenceWatcher::availabilityColor() const
{
	return Presence::availabilityToColor(availability());
}

QString UserPresenceWatcher::statusText() const
{
	if (const auto presence = PresenceCache::instance()->presence(m_jid, m_resource))
		return presence->statusText();
	return {};
}

QString UserPresenceWatcher::jid() const
{
	return m_jid;
}

void UserPresenceWatcher::setJid(const QString &jid)
{
	if (m_jid != jid) {
		m_jid = jid;
		emit jidChanged();

		if (m_resourceAutoPicked)
			autoPickResource();
	}
}

QString UserPresenceWatcher::resource() const
{
	return m_resource;
}

void UserPresenceWatcher::handlePresenceChanged(PresenceCache::ChangeType type,
	const QString &jid,
	const QString &resource)
{
	if (m_jid == jid) {
		if (m_resourceAutoPicked) {
			// no matter if a new device has connected, a device has
			// disconnected or a device's presence has been updated, we
			// always need to reselect the device to get the most important
			// presence:
			const auto resourceChanged = autoPickResource();

			if (!resourceChanged && type == PresenceCache::Updated && m_resource == resource) {
				// If the resource didn't change, the notify signals won't
				// be emitted. However, if the current resource's presence
				// was updated, we still want those signals to be emitted.
				emit presencePropertiesChanged();
			}
		} else if (m_resource == resource) {
			// the resource is fixed: we only need to call the updated-
			// signals since the resource can't change
			emit presencePropertiesChanged();
		}
	}
}

void UserPresenceWatcher::handlePresencesCleared()
{
	if (m_resourceAutoPicked) {
		m_resource.clear();
		emit resourceChanged();
	}
	emit presencePropertiesChanged();
}

/**
 * Sets the resource and calls the updated-signals for all properties, if the resource
 * changed.
 *
 * @param autoPicked Whether the resource was picked automatically (@c true) or should be
 * fixed to this value (@c false).
 *
 * @return @c true, if the resource has actually changed
 */
bool UserPresenceWatcher::setResource(const QString &resource, bool autoPicked)
{
	m_resourceAutoPicked = autoPicked;
	if (m_resource != resource) {
		// resource was explicitly set to empty -> switch back to auto picking
		if (!autoPicked && resource.isNull()) {
			return autoPickResource();
		}

		m_resource = resource;
		emit resourceChanged();

		emit presencePropertiesChanged();
		return true;
	}
	return false;
}

/**
 * Automatically picks the resource with the most important presence.
 *
 * @return @c true, if the selected resource has changed
 */
bool UserPresenceWatcher::autoPickResource()
{
	if (const auto resource = PresenceCache::instance()->pickIdealResource(m_jid);
		!resource.isNull()) {
		return setResource(resource, true);
	} else if (!m_resource.isNull()) {
		// currently no resource available :'(
		return setResource({}, true);
	}
	return false;
}
