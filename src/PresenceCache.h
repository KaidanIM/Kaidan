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

#ifndef PRESENCECACHE_H
#define PRESENCECACHE_H

#include <QObject>
#include <QMap>
#include <QQmlListProperty>
#include <gloox/presence.h>

/**
 * @class EntityPresence Holds a presence for a single XMPP entity (JID with resource)
 */
class EntityPresence : public QObject
{
	Q_OBJECT
	Q_PROPERTY(quint8 type READ getType NOTIFY typeChanged)
	Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)

public:
	EntityPresence(gloox::Presence::PresenceType type, QString status)
	 : type(type), status(status)
	{
	}

	quint8 getType() const
	{
		return (quint8) type;
	}

	void setType(quint8 type)
	{
		this->type = (gloox::Presence::PresenceType) type;
		emit typeChanged();
	}

	QString getStatus() const
	{
		return status;
	}

	void setStatus(QString status)
	{
		this->status = status;
		emit statusChanged();
	}

signals:
	void typeChanged();
	void statusChanged();

private:
	gloox::Presence::PresenceType type;
	QString status;
};

/**
 * @class ContactPresences Holds presences for each resource of a JID
 */
class ContactPresences : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<QString> resources READ getResources NOTIFY resourcesChanged)

public:
	ContactPresences(QString jid, QObject *parent = nullptr);
	~ContactPresences();

	QQmlListProperty<QString> getResources();

	Q_INVOKABLE EntityPresence* getResourcePresence(QString resource)
	{
		if (!presences.contains(resource))
			presences[resource] = new EntityPresence(gloox::Presence::Unavailable, QString());
		return presences[resource];
	}

	Q_INVOKABLE QString getDefaultResource()
	{
		if (!presences.empty())
			return presences.firstKey();
		else
			return "";
	}

	Q_INVOKABLE EntityPresence* getDefaultPresence()
	{
		if (!presences.empty())
			return presences.first();
		else
			return defaultPresence;
	}

signals:
	void resourcesChanged();

private:
	QMap<QString, EntityPresence*> presences;
	QString jid;
	EntityPresence *defaultPresence;
};

/**
 * @class PresenceCache A cache for presence holders for certain JIDs
 */
class PresenceCache : public QObject
{
	Q_OBJECT

public:
	/**
	 * Default constructor
	 */
	PresenceCache(QObject *parent = nullptr);

	/**
	 * Destructor
	 */
	~PresenceCache();

	/**
	 * Get presences of a certain JID
	 * @param jid Account address of the presences
	 */
	Q_INVOKABLE ContactPresences* getPresences(QString jid)
	{
		if (!presences.contains(jid))
			presences[jid] = new ContactPresences(jid);
		return presences[jid];
	}

	Q_INVOKABLE QString getDefaultStatus(QString jid);

	Q_INVOKABLE quint8 getDefaultPresType(QString jid);

signals:
	void presenceArrived(QString jid, QString resource,
	                     gloox::Presence::PresenceType type, QString status);

	void presenceChanged(QString jid);

private slots:
	void updatePresence(QString jid, QString resource,
	                    gloox::Presence::PresenceType type, QString status);

private:
	QMap<QString, ContactPresences*> presences;
};

#endif // PRESENCECACHE_H
