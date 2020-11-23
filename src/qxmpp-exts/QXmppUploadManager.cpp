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

#include "QXmppUploadManager.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QXmppHttpUpload::QXmppHttpUpload(QXmppUploadManager *manager)
    : QXmppLoggable(manager),
      m_manager(manager),
      m_netManager(new QNetworkAccessManager(this)),
      m_putReply(nullptr)
{
}

QXmppHttpUpload::QXmppHttpUpload(const QXmppHttpUpload &upload)
    : QXmppLoggable(upload.m_manager),
      m_manager(upload.m_manager),
      m_customFileName(upload.m_customFileName),
      m_fileInfo(upload.m_fileInfo),
      m_requestId(upload.m_requestId),
      m_requestError(upload.m_requestError),
      m_slot(upload.m_slot),
      m_bytesSent(upload.m_bytesSent),
      m_bytesTotal(upload.m_bytesTotal),
      m_uploadError(upload.m_uploadError),
      m_netManager(new QNetworkAccessManager(this)),
      m_putReply(nullptr)
{
}

QXmppHttpUpload::~QXmppHttpUpload()
{
}

QXmppHttpUpload& QXmppHttpUpload::operator=(const QXmppHttpUpload &other)
{
    if (this != &other) {
        m_manager = other.m_manager;
        m_customFileName = other.m_customFileName;
        m_fileInfo = other.m_fileInfo;
        m_requestId = other.m_requestId;
        m_requestError = other.m_requestError;
        m_slot = other.m_slot;
        m_bytesSent = other.m_bytesSent;
        m_bytesTotal = other.m_bytesTotal;
        m_uploadError = other.m_uploadError;
    }
    return *this;
}

bool operator==(const QXmppHttpUpload &l, const QXmppHttpUpload &r)
{
    return l.id() == r.id() && l.customFileName() == r.customFileName() &&
           l.fileInfo() == r.fileInfo() && l.requestId() == r.requestId();
}

int QXmppHttpUpload::id() const
{
    return m_id;
}

void QXmppHttpUpload::setId(int id)
{
    m_id = id;
}

QString QXmppHttpUpload::requestId() const
{
    return m_requestId;
}

void QXmppHttpUpload::setRequestId(const QString &requestId)
{
    m_requestId = requestId;
}

QString QXmppHttpUpload::customFileName() const
{
    return m_customFileName;
}

void QXmppHttpUpload::setCustomFileName(const QString &customFileName)
{
    m_customFileName = customFileName;
}

QFileInfo QXmppHttpUpload::fileInfo() const
{
    return m_fileInfo;
}

void QXmppHttpUpload::setFileInfo(const QFileInfo &fileInfo)
{
    m_fileInfo = fileInfo;
    m_bytesTotal = m_fileInfo.size();
}

QXmppHttpUploadSlotIq QXmppHttpUpload::slot() const
{
    return m_slot;
}

void QXmppHttpUpload::setSlot(const QXmppHttpUploadSlotIq &slot)
{
    m_slot = slot;
}

QXmppHttpUploadRequestIq QXmppHttpUpload::requestError() const
{
    return m_requestError;
}

void QXmppHttpUpload::setRequestError(const QXmppHttpUploadRequestIq &requestError)
{
    m_requestError = requestError;
}

bool QXmppHttpUpload::started() const
{
    return m_started;
}

/// Returns the number of bytes sent to the server.

qint64 QXmppHttpUpload::bytesSent() const
{
    return m_bytesSent;
}

qint64 QXmppHttpUpload::bytesTotal() const
{
    return m_bytesTotal;
}

void QXmppHttpUpload::handleProgressed(qint64 sent, qint64 total)
{
    if (m_bytesSent != sent) {
        m_bytesSent = sent;
        emit bytesSentChanged();
    }
    if (m_bytesTotal != total) {
        m_bytesTotal = total;
        emit bytesTotalChanged();
    }
}

/// Starts uploading the file. It assumes that a valid slot and fileInfo are set.

void QXmppHttpUpload::startUpload()
{
    m_started = true;
    QNetworkRequest request(m_slot.putUrl());

    // include the file's content-type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(m_fileInfo);
    if (mimeType.isValid() && !mimeType.isDefault())
        request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType.name());

    // other header fields
    const auto headers = m_slot.putHeaders();
    const auto headerKeys = headers.keys();
    for (const auto &name : headerKeys)
        request.setRawHeader(name.toUtf8(), headers.value(name).toUtf8());

    // open file
    QFile *file = new QFile(m_fileInfo.filePath());
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadFailed(QNetworkReply::NoError);
        return;
    }
    // start put request and connect slots
    m_putReply = m_netManager->put(request, file);

    connect(m_putReply, &QNetworkReply::finished, this, &QXmppHttpUpload::uploadFinished);
    connect(m_putReply, &QNetworkReply::uploadProgress, this, &QXmppHttpUpload::handleProgressed);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_putReply, &QNetworkReply::errorOccurred, this, &QXmppHttpUpload::uploadFailed);
#else
    connect(m_putReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &QXmppHttpUpload::uploadFailed);
#endif

    // delete file object after upload
    connect(m_putReply, &QNetworkReply::finished, this, [=] () {
        file->deleteLater();
        m_started = false;
    });
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_putReply, &QNetworkReply::errorOccurred, this, [=] () {
#else
    connect(m_putReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, [=] () {
#endif
        file->deleteLater();
        m_started = false;
    });
}

void QXmppHttpUpload::abort()
{
    if (m_started) {
        m_putReply->abort();
        emit uploadFailed(QNetworkReply::NoError);
        m_started = false;
    }
}

QXmppUploadManager::QXmppUploadManager()
    : QXmppUploadRequestManager()
{
    connect(this, &QXmppUploadManager::slotReceived,
            this, &QXmppUploadManager::handleSlot);
    connect(this, &QXmppUploadManager::requestFailed,
            this, &QXmppUploadManager::handleRequestError);
}

QXmppUploadManager::~QXmppUploadManager()
{
    for (QXmppHttpUpload *upload : qAsConst(m_uploads))
        upload->abort();
}

/// Requests an upload slot and starts uploading the file
///
/// \param file The file to be uploaded.
/// \param allowParallel Enable to upload this file even while another file is uploading.
/// \param customFileName Changes the file name on the server.
/// \return Returns the upload's id. The id is unique per session (as long as the UploadManager
/// exists).

const QXmppHttpUpload* QXmppUploadManager::uploadFile(const QFileInfo &file, bool allowParallel, QString customFileName)
{
    auto *upload = new QXmppHttpUpload(this);
    upload->setFileInfo(file);
    upload->setCustomFileName(customFileName);
    upload->setId(m_nextJobId++);

    // connect signals
    connect(upload, &QXmppHttpUpload::uploadFinished,
            this, &QXmppUploadManager::handleUploadFinished);
    connect(upload, &QXmppHttpUpload::uploadFailed,
            this, &QXmppUploadManager::handleUploadFailed);

    m_uploads.append(upload);

    if (m_runningJobs == 0 || allowParallel)
        startNextUpload();

    return upload;
}

void QXmppUploadManager::startNextUpload()
{
    if (m_uploads.isEmpty())
        return;

    // If an upload is started in parallel, runningJobs will be larger than 0.
    // In this case we'll start the last added upload, otherwise the next in the queue.
    QXmppHttpUpload *upload = m_runningJobs > 0 ? m_uploads.last() : m_uploads.first();
    m_runningJobs++;

    const auto fileName = upload->customFileName().isEmpty() ? upload->fileInfo().fileName()
                                                             : upload->customFileName();
    QString reqId = requestUploadSlot(upload->fileInfo(), fileName, {});
    upload->setRequestId(reqId);

    if (reqId.isEmpty()) {
        m_runningJobs--;
        m_uploads.removeAll(upload);
        emit uploadFailed(upload);
        upload->deleteLater();

        // start next upload, if this wasn't a parallel upload
        if (m_runningJobs == 0)
            startNextUpload();
        return;
    }
}

/// Handles requested upload slots

void QXmppUploadManager::handleSlot(const QXmppHttpUploadSlotIq &slot)
{
    for (QXmppHttpUpload *upload : qAsConst(m_uploads)) {
        if (upload->requestId() != slot.id())
            continue;

        if (!m_httpAllowed && (slot.getUrl().scheme() == "http" ||
                        slot.putUrl().scheme() == "http")) {
            m_runningJobs--;
            m_uploads.removeAll(upload);
            emit uploadFailed(upload);
            delete upload;
            return;
        }
        upload->setSlot(slot);
        upload->startUpload();

        break;
    }
}

/// Handles upload slot request errors

void QXmppUploadManager::handleRequestError(const QXmppHttpUploadRequestIq &request)
{
    for (QXmppHttpUpload *upload : qAsConst(m_uploads)) {
        if (upload->requestId() == request.id()) {
            m_runningJobs--;
            m_uploads.removeAll(upload);

            upload->setRequestError(request);
            emit uploadFailed(upload);
            delete upload;
            return;
        }
    }
}

/// Handles finished uploads

void QXmppUploadManager::handleUploadFinished()
{
    m_runningJobs--;
    auto *upload = static_cast<QXmppHttpUpload*>(sender());
    if (upload) {
        m_uploads.removeAll(upload);
        emit uploadSucceeded(upload);
        delete upload;
    }
}

/// Handles upload errors

void QXmppUploadManager::handleUploadFailed(QNetworkReply::NetworkError code)
{
    m_runningJobs--;
    qDebug() << "Upload failed" << code;
}

/// Returns whether unencrypted connections are allowed

bool QXmppUploadManager::httpAllowed()
{
    return m_httpAllowed;
}

/// Set whether unencrypted connections are allowed. By default only HTTPS connections are allowed.
/// Only HTTPS connections are XEP-compliant, but there is a minority of HTTP services, though.

void QXmppUploadManager::setHttpAllowed(bool httpAllowed)
{
    m_httpAllowed = httpAllowed;
}
