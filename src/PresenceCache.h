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

#pragma once

// std
#include <optional>
// Qt
#include <QObject>
// QXmpp
#include <QXmppPresence.h>

class Presence
{
	Q_GADGET
public:
	enum Availability { Offline, Online, Away, XA, DND, Chat };
	Q_ENUM(Availability)

	static constexpr Availability availabilityFromAvailabilityStatusType(
		QXmppPresence::AvailableStatusType type)
	{
		switch (type) {
		case QXmppPresence::Online:
			return Online;
		case QXmppPresence::Away:
			return Away;
		case QXmppPresence::XA:
			return XA;
		case QXmppPresence::DND:
			return DND;
		case QXmppPresence::Chat:
			return Chat;
		default:
			return Offline;
		}
	}
};

/**
 * @class PresenceCache A cache for presence holders for certain JIDs
 */
class PresenceCache : public QObject
{
	Q_OBJECT

public:
	enum ChangeType : quint8 {
		Connected,
		Disconnected,
		Updated,
	};
	Q_ENUM(ChangeType)

	PresenceCache(QObject *parent = nullptr);
	~PresenceCache();

	static PresenceCache *instance()
	{
		return s_instance;
	}

	QString pickIdealResource(const QString &jid);
	std::optional<QXmppPresence> presence(const QString &jid, const QString &resource);

public slots:
	/**
	 * Updates the presence cache, it will ignore subscribe presences
	 */
	void updatePresence(const QXmppPresence &presence);

	/**
	 * Clears all cached presences.
	 */
	void clear();

signals:
	/**
	 * Notifies about changed presences
	 */
	void presenceChanged(PresenceCache::ChangeType type, const QString &jid, const QString &resource);
	void presencesCleared();

private:
	constexpr qint8 availabilityPriority(QXmppPresence::AvailableStatusType type);
	bool presenceMoreImportant(const QXmppPresence &a, const QXmppPresence &b);

	QMap<QString, QMap<QString, QXmppPresence>> m_presences;

	static PresenceCache *s_instance;
};

class UserPresenceWatcher : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString jid READ jid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString resource READ resource WRITE setResource NOTIFY resourceChanged)
	Q_PROPERTY(Presence::Availability availability READ availability NOTIFY presencePropertiesChanged)
	Q_PROPERTY(QString statusText READ statusText NOTIFY presencePropertiesChanged)

public:
	explicit UserPresenceWatcher(QObject *parent = nullptr);

	QString jid() const;
	void setJid(const QString &jid);

	QString resource() const;
	bool setResource(const QString &resource, bool autoPicked = false);

	Presence::Availability availability() const;
	QString statusText() const;

	Q_SIGNAL void jidChanged();
	Q_SIGNAL void resourceChanged();
	Q_SIGNAL void presencePropertiesChanged();

private:
	Q_SLOT void handlePresenceChanged(PresenceCache::ChangeType type,
		const QString &jid,
		const QString &resource);
	Q_SLOT void handlePresencesCleared();

	bool autoPickResource();

	QString m_jid;
	QString m_resource;
	bool m_resourceAutoPicked;
};
