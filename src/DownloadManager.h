/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QFile>
#include <QThread>
#include <QUrl>
#include <QMap>

class Kaidan;
class TransferCache;
class MessageModel;
class QNetworkAccessManager;

class DownloadJob : public QObject
{
	Q_OBJECT
public:
	DownloadJob(QString msgId,
	            QUrl source,
	            QString filePath,
	            QNetworkAccessManager *netMngr,
	            TransferCache *transferCache,
	            Kaidan *kaidan);

	QString downloadLocation() const;

signals:
	void startDownloadRequested();
	void finished();
	void failed();

private slots:
	void startDownload();

private:
	QString msgId;
	QUrl source;
	QString filePath;
	QNetworkAccessManager *netMngr;
	TransferCache *transferCache;
	Kaidan *kaidan;
	QFile file;
};

class DownloadThread : public QThread
{
	Q_OBJECT
public:
	DownloadThread()
	{
		setObjectName("DownloadManager");
	}

protected:
	void run() override
	{
		exec();
	}
};

class DownloadManager : public QObject
{
	Q_OBJECT
public:
	DownloadManager(Kaidan *kaidan, TransferCache *transferCache,
	                MessageModel *model, QObject *parent = nullptr);
	~DownloadManager();

signals:
	void startDownloadRequested(const QString msgId, const QString url);
	void abortDownloadRequested(const QString msgId);

public slots:
	void startDownload(const QString &msgId, const QString &url);
	void abortDownload(const QString &msgId);

private:
	DownloadThread *thread;
	QNetworkAccessManager *netMngr;
	Kaidan *kaidan;
	TransferCache *transferCache;
	MessageModel *model;

	QMap<QString, DownloadJob*> downloads;
};

#endif // DOWNLOADMANAGER_H
