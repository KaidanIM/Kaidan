/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2018 Kaidan developers and contributors
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

#include "LinkPreviewFetcher.h"

// Qt
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>

// Json
// #include <nlohmann/json.hpp>
// using json = nlohmann::json;

static const QString LINK_PREVIEW_BASE_URL = "https://previews.ada.support/?url=";

LinkPreviewFetcher::LinkPreviewFetcher(QObject *parent): QObject(parent)
{
// create previews directory, if it doesn't exists
QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
if (!cacheDir.exists("link-previews"))
	cacheDir.mkpath("link-previews");
}

LinkPreviewFetcher::~LinkPreviewFetcher()
{
}

void LinkPreviewFetcher::fetchPreview(QString url)
{
	// Calculate hash of url
	QString urlHash = QString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Sha1).toHex());

	QString localCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + 
		QDir::separator() + QString("link-previews") +  QDir::separator() + QString(urlHash);

	QFileInfo localCacheInfo(localCachePath);

	if (localCacheInfo.exists() && localCacheInfo.isFile()) {
		qDebug() << "Anscheinend wird der Kack hier auch ausgeführt!";

		QFile file(localCachePath);

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;

		QJsonDocument previewDoc;
		previewDoc.fromBinaryData(file.readAll());

		QJsonObject previewObj = previewDoc.object();

		qDebug() << previewObj.keys();

		file.close();
	} else {
		QString linkPreviewUrl = LINK_PREVIEW_BASE_URL + url;

		QNetworkAccessManager *manager = new QNetworkAccessManager(this);

		// when the manager finishes downloading, trigger `handleJsonData`
		connect(manager, &QNetworkAccessManager::finished, [=](QNetworkReply *reply) {
			handleJsonData(reply, urlHash, localCachePath);
		});

		// manager will automatically trigger `handleJsonData`, when finished
		QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(linkPreviewUrl)));
	}
}

void LinkPreviewFetcher::handleJsonData(QNetworkReply *reply, QString urlHash, QString localCachePath)
{
	if (reply->error()) {
		qDebug() << "Error while trying to fetch link preview" << reply->errorString();
			return;
	} else {
		qDebug() << "Successfully got link preview";

		// Open file (named after url hash) to save json data in
		QFile file(localCachePath);

		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		// Write downloaded data into file
		QTextStream out(&file);
		out << reply->readAll();
	}
}

// AvatarFileStorage::AvatarFileStorage(QObject *parent) : QObject(parent)
// {
// 	// create avatar directory, if it doesn't exists
// 	QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
// 	if (!cacheDir.exists("link-previews"))
// 		cacheDir.mkpath("link-previews");
// }
