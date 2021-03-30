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

#include "DownloadManager.h"

// Qt
#include <QDir>
#include <QNetworkReply>
#include <QStandardPaths>
// Kaidan
#include "Kaidan.h"
#include "MessageModel.h"
#include "TransferCache.h"

DownloadManager::DownloadManager(TransferCache *transferCache, QObject *parent)
	: QObject(parent),
	  m_netMngr(new QNetworkAccessManager(this)),
	  m_transferCache(transferCache)
{
	connect(this, &DownloadManager::startDownloadRequested,
	        this, &DownloadManager::startDownload);
	connect(this, &DownloadManager::abortDownloadRequested, this, &DownloadManager::abortDownload);
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::startDownload(const QString &msgId, const QString &url)
{
	// don't download the same file twice and in parallel
	if (m_downloads.contains(msgId)) {
		qWarning() << "Tried to download a file that is currently being "
		              "downloaded.";
		return;
	}

	// we want to save files to 'Downloads/Kaidan/'
	QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +
	                  QDir::separator() + APPLICATION_DISPLAY_NAME + QDir::separator();

	auto *dl = new DownloadJob(msgId, QUrl(url), dirPath, m_netMngr, m_transferCache);
	m_downloads[msgId] = dl;

	connect(dl, &DownloadJob::finished, this, [=]() {
		const QString &mediaLocation = dl->downloadLocation();
		emit MessageModel::instance()->updateMessageRequested(msgId, [=](Message &msg) {
			msg.setMediaLocation(mediaLocation);
		});

		abortDownload(msgId);
	});
	connect(dl, &DownloadJob::failed, this, [=]() {
		abortDownload(msgId);
	});

	emit dl->startDownloadRequested();
}

void DownloadManager::abortDownload(const QString &msgId)
{
	auto *job = m_downloads.value(msgId);
	job->deleteLater();
	m_downloads.remove(msgId);

	emit m_transferCache->removeJobRequested(msgId);
}

DownloadJob::DownloadJob(const QString &msgId,
	const QUrl &source,
	const QString &filePath,
	QNetworkAccessManager *netMngr,
	TransferCache *transferCache)
	: QObject(nullptr),
	  m_msgId(msgId),
	  m_source(source),
	  m_filePath(filePath),
	  m_netMngr(netMngr),
	  m_transferCache(transferCache)
{
	connect(this, &DownloadJob::startDownloadRequested,
	        this, &DownloadJob::startDownload);
}

void DownloadJob::startDownload()
{
	QDir dlDir(m_filePath);
	if (!dlDir.exists())
		dlDir.mkpath(".");

	// don't override other files
	m_file.setFileName(m_filePath + m_source.fileName());
	int counter = 1;
	while (m_file.exists()) {
		m_file.setFileName(
			m_filePath + m_source.fileName() + "-" + QString::number(counter++));
	}

	if (!m_file.open(QIODevice::WriteOnly)) {
		qWarning() << "Could not open file for writing:" << m_file.errorString();
		emit Kaidan::instance()->passiveNotificationRequested(
			tr("Could not save file: %1").arg(m_file.errorString()));
		emit failed();
		return;
	}

	QNetworkRequest request(m_source);
	QNetworkReply *reply = m_netMngr->get(request);

	emit m_transferCache->addJobRequested(m_msgId, 0);

	connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
		emit m_transferCache->setJobProgressRequested(m_msgId, bytesReceived, bytesTotal);
	});
	connect(reply, &QNetworkReply::finished, this, [=]() {
		emit m_transferCache->removeJobRequested(m_msgId);
		emit finished();
	});
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect(reply, &QNetworkReply::errorOccurred, this, [=]() {
#else
	connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [=]() {
#endif
		emit m_transferCache->removeJobRequested(m_msgId);
		qWarning() << "Couldn't download file:" << reply->errorString();
		emit Kaidan::instance()->passiveNotificationRequested(
			tr("Download failed: %1").arg(reply->errorString()));
		emit finished();
	});
	connect(reply, &QNetworkReply::readyRead, this, [=]() {
		m_file.write(reply->readAll());
	});
}

QString DownloadJob::downloadLocation() const
{
	return m_file.fileName();
}
