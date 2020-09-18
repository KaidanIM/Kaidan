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

#include "TransferCache.h"
#include <QMutexLocker>

TransferJob::TransferJob(qint64 bytesTotal)
	: m_progress(0.0), m_bytesSent(0), m_bytesTotal(bytesTotal)
{
}

void TransferJob::setProgress(qreal progress)
{
	if (m_progress == progress)
		return;
	m_progress = progress;
	emit progressChanged();
}

void TransferJob::setBytesSent(qint64 bytesSent)
{
	if (m_bytesSent == bytesSent)
		return;
	m_bytesSent = bytesSent;
	emit bytesSentChanged();

	if (m_bytesTotal != 0)
		setProgress(qreal(bytesSent) / qreal(m_bytesTotal));
}

void TransferJob::setBytesTotal(qint64 bytesTotal)
{
	if (m_bytesTotal == bytesTotal)
		return;
	m_bytesTotal = bytesTotal;
	emit bytesTotalChanged();

	if (bytesTotal != 0)
		setProgress(qreal(m_bytesSent) / qreal(bytesTotal));
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
	QMutexLocker locker(&m_mutex);
}

void TransferCache::addJob(const QString& msgId, qint64 bytesTotal)
{
	QMutexLocker locker(&m_mutex);
	m_uploads.insert(msgId, new TransferJob(bytesTotal));
	locker.unlock();

	emit jobsChanged();
}

void TransferCache::removeJob(const QString& msgId)
{
	QMutexLocker locker(&m_mutex);
	auto upload = m_uploads.take(msgId);
	upload->deleteLater();
	locker.unlock();

	emit jobsChanged();
}

bool TransferCache::hasUpload(QString msgId) const
{
	QMutexLocker locker(&m_mutex);
	return m_uploads.contains(msgId);
}

TransferJob* TransferCache::jobByMessageId(QString msgId) const
{
	QMutexLocker locker(&m_mutex);
	TransferJob *job = m_uploads.value(msgId);
	if (job == nullptr)
		return m_emptyJob;
	return job;
}

void TransferCache::setJobProgress(const QString &msgId, qint64 bytesSent, qint64 bytesTotal)
{
	TransferJob* job = jobByMessageId(msgId);

	QMutexLocker locker(&m_mutex);
	job->setBytesTotal(bytesTotal);
	job->setBytesSent(bytesSent);
}

void TransferCache::setJobBytesSent(const QString &msgId, qint64 bytesSent)
{
	TransferJob* job = jobByMessageId(msgId);

	QMutexLocker locker(&m_mutex);
	job->setBytesSent(bytesSent);
}
