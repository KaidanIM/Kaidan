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

#include "AvatarFileStorage.h"

#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDebug>

AvatarFileStorage::AvatarFileStorage()
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
	}

	// abort if the avatar with this hash is already saved
	if (hasAvatarHash(result.hash))
		return result;

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

	return result;
}

QString AvatarFileStorage::getAvatarPath(const QString &hash) const
{
	return QStandardPaths::locate(QStandardPaths::CacheLocation, QString("avatars") +
		QDir::separator() + hash, QStandardPaths::LocateFile);
}

QString AvatarFileStorage::getHashForJid(const QString& jid) const
{
	return jidAvatarMap[jid];
}

QString AvatarFileStorage::getAvatarPathForJid(const QString& jid) const
{
	return getAvatarPath(getHashForJid(jid));
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
