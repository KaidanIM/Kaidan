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

#ifndef TRANSFERCACHE_H
#define TRANSFERCACHE_H

#include <QObject>
#include <QMap>
#include <QMutex>

/**
 * @class TransferJob Upload/download progress information
 */
class TransferJob : public QObject
{
	Q_OBJECT
	Q_PROPERTY(qreal progress MEMBER progress NOTIFY progressChanged)
	Q_PROPERTY(qint64 bytesSent MEMBER bytesSent NOTIFY bytesSentChanged)
	Q_PROPERTY(qint64 bytesTotal MEMBER bytesTotal NOTIFY bytesTotalChanged)

public:
	TransferJob(qint64 bytesTotal);

	Q_INVOKABLE void setProgress(qreal progress);
	Q_INVOKABLE void setBytesSent(qint64 bytesSent);
	Q_INVOKABLE void setBytesTotal(qint64 bytesTotal);

signals:
	void progressChanged();
	void bytesSentChanged();
	void bytesTotalChanged();

private:
	qreal progress;
	qint64 bytesSent;
	qint64 bytesTotal;
};

/**
 * @class TransferCache Caching upload and download meta.
 *
 * This class is thread-safe.
 */
class TransferCache : public QObject
{
	Q_OBJECT

public:
	TransferCache(QObject *parent = nullptr);
	~TransferCache();

	/**
	 * Returns true if the cache contains an upload for the specified message id.
	 */
	Q_INVOKABLE bool hasUpload(QString msgId) const;

	/**
	 * Returns the upload associated with the message id (used for progress)
	 */
	Q_INVOKABLE TransferJob* jobByMessageId(QString msgId) const;

public slots:
	Q_INVOKABLE void addJob(const QString &msgId, qint64 bytesTotal);
	Q_INVOKABLE void removeJob(const QString& msgId);
	Q_INVOKABLE void setJobBytesSent(const QString& msgId, qint64 bytesSent);
	Q_INVOKABLE void setJobProgress(const QString& msgId, qint64 bytesSent,
	                                qint64 bytesTotal);

signals:
	/**
	 * @brief Emitted when a job was added or removed; useful for getting notified
	 * about changes of hasUpload().
	 */
	void jobsChanged();
	void addJobRequested(const QString& msgId, qint64 bytesTotal);
	void removeJobRequested(const QString& msgId);
	void setJobBytesSentRequested(const QString& msgId, qint64 bytesSent);
	void setJobProgressRequested(const QString& msgId, qint64 bytesSent,
	                             qint64 bytesTotal);

private:
	QMap<QString, TransferJob*> uploads;
	TransferJob* emptyJob;

	mutable QMutex mutex;
};

#endif // TRANSFERCACHE_H
