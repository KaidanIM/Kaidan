/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

#ifndef AVATARFILESTORAGE_H
#define AVATARFILESTORAGE_H

#include <QObject>
#include <QString>
#include <QMap>

class AvatarFileStorage : public QObject
{
	Q_OBJECT

public:
	AvatarFileStorage(QObject *parent = 0);

	struct AddAvatarResult {
		/* SHA1 HEX Hash */
		QString hash;
		/* If the hash for the JID has changed */
		bool hasChanged = false;
		/* If the avater was new and had to be written onto the disk */
		bool newWritten = false;
	};

	/**
	 * Add a new avatar in binary form that will be saved in a cache location
	 *
	 * @param jid The JID the avatar belongs to
	 * @param avatar The binary avatar (not in base64)
	 */
	AddAvatarResult addAvatar(const QString &jid, const QByteArray &avatar);

	/**
	 * Clears the user's avatar
	 */
	void clearAvatar(QString &jid);

	/**
	 * Deletes the avatar with this hash, if it isn't used anymore
	 */
	void cleanUp(QString &oldHash);

	/**
	 * Returns the path to the avatar of the JID
	 */
	QString getAvatarPathOfJid(const QString &jid) const;

	/**
	 * Returns true if there an avatar saved for the given hash
	 *
	 * @param hash The SHA1 hash of the binary avatar
	 */
	bool hasAvatarHash(const QString &hash) const;

	/**
	 * Returns the path to a given hash
	 */
	QString getAvatarPath(const QString &hash) const;

	/**
	 * Returns the hash of the avatar of the JID
	 */
	Q_INVOKABLE QString getHashOfJid(const QString &jid) const;

	/**
	 * Returns a file URL to the avatar image of a given JID
	 */
	Q_INVOKABLE QString getAvatarUrl(const QString &jid) const;

signals:
	void avatarIdsChanged();

private:
	void saveAvatarsFile();

	QMap<QString, QString> jidAvatarMap;
};

#endif // AVATARFILESTORAGE_H
