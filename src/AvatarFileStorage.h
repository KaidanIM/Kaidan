/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AVATARFILESTORAGE_H
#define AVATARFILESTORAGE_H

#include <QString>
#include <QMap>

class AvatarFileStorage
{
public:
	AvatarFileStorage();
	~AvatarFileStorage();

	struct AddAvatarResult {
		/* SHA3-256 HEX Hash */
		QString hash;
		/* If the hash for the JID has changed */
		bool hasChanged = false;
		/* If the avater was new and had to be written onto the disk */
		bool newWritten = false;
	};

	bool hasAvatarHash(const QString &hash) const;
	AddAvatarResult addAvatar(const QString &jid, const QByteArray &avatar);
	QString getAvatarPath(const QString &hash) const;
	QString getAvatarPathForJid(const QString &jid) const;
	QString getHashForJid(const QString &jid) const;

private:
	void saveAvatarsFile();

	QMap<QString, QString> jidAvatarMap;
};

#endif // AVATARFILESTORAGE_H
