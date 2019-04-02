/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

#include "TransferCache.h"
#include "qxmpp-exts/QXmppUploadManager.h"
#include <QMutexLocker>

TransferJob::TransferJob(qint64 bytesTotal)
	: QObject(), progress(0.0), bytesSent(0), bytesTotal(bytesTotal)
{
}

void TransferJob::setProgress(qreal progress)
{
	if (this->progress == progress)
		return;
	this->progress = progress;
	emit progressChanged();
}

void TransferJob::setBytesSent(qint64 bytesSent)
{
	if (this->bytesSent == bytesSent)
		return;
	this->bytesSent = bytesSent;
	emit bytesSentChanged();
	if (bytesTotal != 0)
		setProgress((qreal) bytesSent / (qreal) bytesTotal);
}

void TransferJob::setBytesTotal(qint64 bytesTotal)
{
	if (this->bytesTotal == bytesTotal)
		return;
	this->bytesTotal = bytesTotal;
	emit bytesTotalChanged();
	if (bytesTotal != 0)
		setProgress((qreal) bytesSent / (qreal) bytesTotal);
}

TransferCache::TransferCache(QObject *parent)
	: QObject(parent)
{
	connect(this, &TransferCache::addJobRequested, this, &TransferCache::addJob);
	connect(this, &TransferCache::removeJobRequested,
	        this, &TransferCache::removeJob);
	connect(this, &TransferCache::setJobBytesSentRequested,
	        this, &TransferCache::setJobBytesSent);
	connect(this, &TransferCache::setJobProgressRequested,
	        this, &TransferCache::setJobBytesSent);
}

TransferCache::~TransferCache()
{
	// wait for other threads to finish
	QMutexLocker locker(&mutex);
}

void TransferCache::addJob(const QString& msgId, qint64 bytesTotal)
{
	QMutexLocker locker(&mutex);
	uploads.insert(msgId, new TransferJob(bytesTotal));
	locker.unlock();

	emit jobsChanged();
}

void TransferCache::removeJob(const QString& msgId)
{
	QMutexLocker locker(&mutex);
	delete uploads[msgId];
	uploads.remove(msgId);
	locker.unlock();

	emit jobsChanged();
}

bool TransferCache::hasUpload(QString msgId) const
{
	QMutexLocker locker(&mutex);
	return uploads.contains(msgId);
}

TransferJob* TransferCache::jobByMessageId(QString msgId) const
{
	QMutexLocker locker(&mutex);
	TransferJob* job = uploads.value(msgId);
	if (job == nullptr)
		return emptyJob;
	return job;
}

void TransferCache::setJobProgress(const QString &msgId, qint64 bytesSent, qint64 bytesTotal)
{
	TransferJob* job = jobByMessageId(msgId);

	QMutexLocker locker(&mutex);
	job->setBytesTotal(bytesTotal);
	job->setBytesSent(bytesSent);
}

void TransferCache::setJobBytesSent(const QString &msgId, qint64 bytesSent)
{
	TransferJob* job = jobByMessageId(msgId);

	QMutexLocker locker(&mutex);
	job->setBytesSent(bytesSent);
}
