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

#include "QtHttpUploader.h"
#include "gloox-extensions/httpuploadmanager.h"
#include "gloox-extensions/httpuploadslot.h"
// Qt
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
// Kaidan
#include "Globals.h"

UploadFile::UploadFile(QString name, QObject* parent) :
	QFile(name, parent)
{
}

QByteArray UploadFile::read(qint64 maxSize)
{
	QByteArray bytes = QFile::read(maxSize);
	emit bytesRead(bytes);

	return bytes;
}

QtHttpUploader::QtHttpUploader(gloox::HttpUploadManager *manager,
                               QObject *parent)
	: QObject(parent), manager(manager)
{
	manager->registerHttpUploader(this);
}

QtHttpUploader::~QtHttpUploader()
{
}

bool QtHttpUploader::busy()
{
	return runningTasks != 0;
}

void QtHttpUploader::uploadFile(int id, std::string putUrl,
                                gloox::HeaderFieldMap putHeaders,
                                std::string &localPath, std::string &contentType)
{
	// open file for read
	UploadFile *file = new UploadFile(QString::fromStdString(localPath));
	if (!file->open(QIODevice::ReadOnly)) {
		manager->uploadFailed(id, gloox::UploadFileNotFound);
		return;
	}

	// save empty data (will be used for saving hashing progress)
	HashData hashData;
	hashCache[id] = hashData;

	// to not read the file several times, we'll generate the hashes directly
	// while uploading
	connect(file, &UploadFile::bytesRead, [=] (const QByteArray &bytes) {
		processHashes(id, bytes);
	});


	//
	// Create HTTP PUT request
	//

	QNetworkRequest request(QUrl(QString::fromStdString(putUrl)));
	if (!contentType.empty())
		request.setHeader(QNetworkRequest::ContentTypeHeader,
		                  QString::fromStdString(contentType));
	for (auto &field : putHeaders) {
		request.setRawHeader(field.first.c_str(), field.second.c_str());
	}


	// send put request and start uploading
	QNetworkAccessManager *netMngr = new QNetworkAccessManager();
	QNetworkReply *reply = netMngr->put(request, file);
	runningTasks++;


	// handle finished
	connect(reply, &QNetworkReply::finished, [=] () {
		runningTasks--;
		manager->uploadFinished(id);
		// after this was handled by the manager, we can delete the hashes
		hashCache.remove(id);
	});

	// error handling
	connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
	        [=] (QNetworkReply::NetworkError code) {
		runningTasks--;
		manager->uploadFailed(id, gloox::UploadHttpError);

		hashCache.remove(id);
		file->deleteLater();
		netMngr->deleteLater();
		reply->deleteLater();
	});

	// upload progress
	connect(reply, &QNetworkReply::uploadProgress, [=] (qint64 sent, qint64 total) {
		if (total < 0)
			total = 0;

		manager->uploadProgress(id, sent, total);
	});

	// delete everything when finished
	connect(reply, &QNetworkReply::finished, file, &UploadFile::deleteLater);
	connect(reply, &QNetworkReply::finished, netMngr, &QNetworkAccessManager::deleteLater);
	connect(reply, &QNetworkReply::finished, reply, &QNetworkAccessManager::deleteLater);
}

void QtHttpUploader::processHashes(int id, const QByteArray &bytes)
{
	hashCache[id].sha256->addData(bytes);
	hashCache[id].sha3_256->addData(bytes);
}

QtHttpUploader::HashResult QtHttpUploader::getHashResults(int id) const
{
	QtHttpUploader::HashResult result;
	if (!hashCache.contains(id))
		return result;

	result.sha256 = hashCache[id].sha256->result();
	result.sha3_256 = hashCache[id].sha3_256->result();
	return result;
}
