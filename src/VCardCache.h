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

#pragma once

// std
#include <optional>
// Qt
#include <QMutex>
#include <QObject>
// QXmpp
#include <QXmppVCardIq.h>

class VCardCache : public QObject
{
	Q_OBJECT

public:
	VCardCache(QObject* parent = nullptr);

	/**
	 * Returns the vCard for a JID.
	 *
	 * This method is thread-safe.
	 *
	 * @param jid JID for which the vCard is retrieved
	 */
	std::optional<QXmppVCardIq> vCard(const QString &jid) const;

	/**
	 * Sets the vCard for a JID.
	 *
	 * This method is thread-safe.
	 *
	 * @param jid JID to which the vCard belongs
	 * @param vCard vCard being set
	 */
	void setVCard(const QString &jid, const QXmppVCardIq &vCard);

signals:
	/**
	 * Emitted when a vCard changed.
	 *
	 * @param jid JID of the changed vCard
	 */
	void vCardChanged(const QString &jid);

private:
	mutable QMutex m_mutex;

	// mapping from a JID to the vCard of that JID
	QHash<QString, QXmppVCardIq> m_vCards;
};
