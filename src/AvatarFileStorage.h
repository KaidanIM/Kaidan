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
	~AvatarFileStorage();

	struct AddAvatarResult {
		/* SHA3-256 HEX Hash */
		QString hash;
		/* If the hash for the JID has changed */
		bool hasChanged = false;
		/* If the avater was new and had to be written onto the disk */
		bool newWritten = false;
	};

	AddAvatarResult addAvatar(const QString &jid, const QByteArray &avatar);
	QString getAvatarPathOfJid(const QString &jid) const;
	bool hasAvatarHash(const QString &hash) const;
	QString getAvatarPath(const QString &hash) const;

	Q_INVOKABLE QString getHashOfJid(const QString &jid) const;
	Q_INVOKABLE QString getAvatarUrl(const QString &jid) const;

signals:
	void avatarIdsChanged();

private:
	void saveAvatarsFile();

	QMap<QString, QString> jidAvatarMap;
};

#endif // AVATARFILESTORAGE_H
