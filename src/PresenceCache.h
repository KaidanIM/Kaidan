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

#include <QObject>
#include <QMap>
#include <QQmlListProperty>
#include <QXmppPresence.h>

/**
 * @class PresenceCache A cache for presence holders for certain JIDs
 */
class PresenceCache : public QObject
{
	Q_OBJECT

public:
	PresenceCache(QObject *parent = nullptr);

	/**
	 * Return one of the status texts from all resources
	 */
	Q_INVOKABLE QString getStatusText(QString bareJid);

	/**
	 * Returns one of the presence types from all resources
	 */
	Q_INVOKABLE quint8 getPresenceType(QString bareJid);

public slots:
	/**
	 * Updates the presence cache, it will ignore subscribe presences
	 */
	void updatePresence(QXmppPresence presence);

	/**
	 * Clears all cached presences.
	 */
	void clear();

signals:
	/**
	 * Notifies about changed presences
	 */
	void presenceChanged(QString jid);

private:
	QMap<QString, QMap<QString, QXmppPresence>> presences;
};
