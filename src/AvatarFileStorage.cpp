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

#include "AvatarFileStorage.h"

#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>

AvatarFileStorage::AvatarFileStorage(QObject *parent) : QObject(parent)
{
	// create avatar directory, if it doesn't exists
	QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
	if (!cacheDir.exists("avatars"))
		cacheDir.mkpath("avatars");

	// search for the avatar list file (hash <-> jid)
	QString avatarFilePath = getAvatarPath("avatar_list.sha3-256");

	try {
		// check if file was found
		if (avatarFilePath != "") {
			//
			// restore saved avatars
			//

			// open the file; if fails return
			QFile avatarFile(avatarFilePath);
			if (!avatarFile.open(QIODevice::ReadOnly))
				return;

			QTextStream stream(&avatarFile);
			// get the first line
			QString line = stream.readLine();
			while (!line.isNull()) {
				// get hash and jid from line (seperated by a blank)
				QStringList list = line.split(' ', QString::SkipEmptyParts);
				QString hash = list.at(0);
				QString jid = list.at(1);

				// set the hash for the jid
				jidAvatarMap[jid] = hash;

				// read the next line
				line = stream.readLine();
			};
		}
	} catch (...) {
		qDebug() << "[AvatarFileStorage] Error in" << avatarFilePath << "(avatar list file)";
	}
}

AvatarFileStorage::~AvatarFileStorage()
{
}

AvatarFileStorage::AddAvatarResult AvatarFileStorage::addAvatar(const QString &jid, const QByteArray &avatar)
{
	AddAvatarResult result;

	// generate a hexadecimal hash of the raw avatar
	result.hash = QString(QCryptographicHash::hash(avatar, QCryptographicHash::Sha3_256).toHex());
	// set the new hash and the `hasChanged` tag
	if (jidAvatarMap[jid] != result.hash) {
		jidAvatarMap[jid] = result.hash;
		result.hasChanged = true;
		emit avatarIdsChanged();
	}

	// abort if the avatar with this hash is already saved
	// only update GUI, if avatar really has changed
	bool hasAvatar = hasAvatarHash(result.hash);
	if (hasAvatar && result.hasChanged) {
		emit avatarIdsChanged();
		return result;
	} else if (hasAvatar) {
		return result;
	}

	// write the avatar to disk:
	// get the writable cache location for writing
	QFile file(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
		QDir::separator() + QString("avatars") + QDir::separator() + result.hash);
	if (!file.open(QIODevice::WriteOnly))
		return result;

	// write the binary avatar
	file.write(avatar);

	saveAvatarsFile();

	// mark that the avatar is new
	result.newWritten = true;

	emit avatarIdsChanged();
	return result;
}

QString AvatarFileStorage::getAvatarPath(const QString &hash) const
{
	return QStandardPaths::locate(QStandardPaths::CacheLocation, QString("avatars") +
		QDir::separator() + hash, QStandardPaths::LocateFile);
}

QString AvatarFileStorage::getHashOfJid(const QString& jid) const
{
	return jidAvatarMap[jid];
}

QString AvatarFileStorage::getAvatarPathOfJid(const QString& jid) const
{
	return getAvatarPath(getHashOfJid(jid));
}

QString AvatarFileStorage::getAvatarUrl(const QString &jid) const
{
	return QString("file://") + getAvatarPathOfJid(jid);
}

bool AvatarFileStorage::hasAvatarHash(const QString& hash) const
{
	return !getAvatarPath(hash).isNull();
}

void AvatarFileStorage::saveAvatarsFile()
{
	QFile file(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
		QDir::separator() + QString("avatars") + QDir::separator() + QString("avatar_list.sha3-256"));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	for (const auto jid : jidAvatarMap.keys())
		/*     < HASH >            < JID >  */
		out << jidAvatarMap[jid] << " " << jid << "\n";
}
