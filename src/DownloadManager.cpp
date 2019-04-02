/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

// Kaidan
#include "DownloadManager.h"
#include "Kaidan.h"
#include "TransferCache.h"
#include "MessageModel.h"
#include "Globals.h"
// Qt
#include "QDir"
#include "QStandardPaths"
#include "QNetworkRequest"
#include "QNetworkReply"
#include "QNetworkAccessManager"

DownloadManager::DownloadManager(Kaidan *kaidan, TransferCache *transferCache,
                                 MessageModel *model, QObject *parent)
        : QObject(parent), thread(new DownloadThread()),
          netMngr(new QNetworkAccessManager), kaidan(kaidan),
          transferCache(transferCache), model(model)
{
	connect(this, &DownloadManager::startDownloadRequested,
	        this, &DownloadManager::startDownload);
	connect(this, &DownloadManager::abortDownloadRequested,
	        this, &DownloadManager::abortDownload);

	connect(kaidan, &Kaidan::downloadMedia, this, &DownloadManager::startDownload);

	netMngr->moveToThread(thread);
	thread->start();
}

DownloadManager::~DownloadManager()
{
	delete netMngr;
	delete thread;
}

void DownloadManager::startDownload(const QString msgId, const QString url)
{
	// don't download the same file twice and in parallel
	if (downloads.keys().contains(msgId)) {
		qWarning() << "Tried to download a file that is currently being "
		              "downloaded.";
		return;
	}

	// we want to save files to 'Downloads/Kaidan/'
	QString dirPath =
	        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
	        + QDir::separator() + APPLICATION_DISPLAY_NAME + QDir::separator();

	DownloadJob *dl = new DownloadJob(msgId, QUrl(url), dirPath, netMngr,
	                                  transferCache, kaidan);
	dl->moveToThread(thread);
	downloads[msgId] = dl;

	connect(dl, &DownloadJob::finished, this, [this, dl, msgId]() {
		MessageModel::Message msgUpdate;
		msgUpdate.mediaLocation = dl->downloadLocation();
		emit model->updateMessageRequested(msgId, msgUpdate);

		abortDownload(msgId);
	});
	connect(dl, &DownloadJob::failed, this, [this, msgId]() {
		abortDownload(msgId);
	});

	emit dl->startDownloadRequested();
}

void DownloadManager::abortDownload(const QString msgId)
{
	DownloadJob *job = downloads.value(msgId);
	if (job != nullptr)
		delete job;
	downloads.remove(msgId);

	emit transferCache->removeJobRequested(msgId);
}

DownloadJob::DownloadJob(QString msgId, QUrl source, QString filePath,
                         QNetworkAccessManager *netMngr,
                         TransferCache *transferCache, Kaidan *kaidan)
        : QObject(nullptr), msgId(msgId), source(source), filePath(filePath),
          netMngr(netMngr), transferCache(transferCache), kaidan(kaidan), file()
{
	connect(this, &DownloadJob::startDownloadRequested,
	        this, &DownloadJob::startDownload);
}

void DownloadJob::startDownload()
{
	QDir dlDir(filePath);
	if (!dlDir.exists())
		dlDir.mkpath(".");

	// don't override other files
	file.setFileName(filePath + source.fileName());
	int counter = 1;
	while (file.exists()) {
		file.setFileName(filePath + source.fileName() + "-"
		                 + QString::number(counter++));
	}

	if (!file.open(QIODevice::WriteOnly)) {
		qWarning() << "Could not open file for writing:"
		           << file.errorString();
		emit kaidan->passiveNotificationRequested(
		        tr("Could not save file: %1").arg(file.errorString()));
		emit failed();
		return;
	}

	QNetworkRequest request(source);
	QNetworkReply *reply = netMngr->get(request);

	emit transferCache->addJobRequested(msgId, 0);

	connect(reply, &QNetworkReply::downloadProgress,
	        this, [this] (qint64 bytesReceived, qint64 bytesTotal) {
		emit transferCache->setJobProgressRequested(msgId, bytesReceived, bytesTotal);
	});
	connect(reply, &QNetworkReply::finished, this, [this] () {
		emit transferCache->removeJobRequested(msgId);
		emit finished();
	});
	connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
	        [this, reply] () {
		emit transferCache->removeJobRequested(msgId);
		qWarning() << "Couldn't download file:" << reply->errorString();
		emit kaidan->passiveNotificationRequested(
		        tr("Download failed: %1").arg(reply->errorString()));
		emit finished();
	});
	connect(reply, &QNetworkReply::readyRead, this, [this, reply](){
		file.write(reply->readAll());
	});
}

QString DownloadJob::downloadLocation() const
{
	return file.fileName();
}
