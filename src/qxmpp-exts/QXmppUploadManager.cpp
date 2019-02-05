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

#include <QMimeDatabase>
#include <QMimeType>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "QXmppUploadManager.h"

QXmppHttpUpload::QXmppHttpUpload(QXmppUploadManager *manager)
    : QXmppLoggable(), m_manager(manager), m_netManager(new QNetworkAccessManager())
{
    connect(this, &QXmppHttpUpload::uploadProgressed, this, &QXmppHttpUpload::handleProgressed);
}

QXmppHttpUpload::QXmppHttpUpload(const QXmppHttpUpload &upload)
    : QXmppLoggable(), m_manager(upload.m_manager), m_customFileName(upload.m_customFileName), 
    m_fileInfo(upload.m_fileInfo), m_requestId(upload.m_requestId),
    m_requestError(upload.m_requestError), m_slot(upload.m_slot), m_bytesSent(upload.m_bytesSent),
    m_uploadError(upload.m_uploadError), m_netManager(new QNetworkAccessManager())
{
}

QXmppHttpUpload::~QXmppHttpUpload()
{
    delete m_netManager;
//     if (m_putReply)
//         delete m_putReply;
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
        m_uploadError = other.m_uploadError;
    }
    return *this;
}

bool operator==(const QXmppHttpUpload &l, const QXmppHttpUpload &r)
{
    return l.id() == r.id() &&
           l.customFileName() == r.customFileName() &&
           l.fileInfo() == r.fileInfo() &&
           l.requestId() == r.requestId() &&
           l.requestError() == r.requestError() &&
           l.slot() == r.slot();
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

/// Returns the number of bytes sent to the server.

qint64 QXmppHttpUpload::progress() const
{
    return m_bytesSent;
}

void QXmppHttpUpload::handleProgressed(qint64 sent, qint64)
{
    m_bytesSent = sent;
}

/// Starts uploading the file. It assumes that a valid slot and fileInfo are set.

void QXmppHttpUpload::startUpload()
{
    QNetworkRequest request(m_slot.putUrl());

    // include the file's content-type
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(m_fileInfo);
    if (mimeType.isValid() && !mimeType.isDefault())
        request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType.name());

    // other header fields
    const QMap<QString, QString> &headers = m_slot.headerFields();
    for (const QString &name : headers.keys())
        request.setRawHeader(name.toUtf8(), headers.value(name).toUtf8());

    // open file
    QFile *file = new QFile(m_fileInfo.absoluteFilePath());
    if (!file->open(QIODevice::ReadOnly)) {
        emit uploadFailed(QNetworkReply::NoError);
        return;
    }
    // start put request and connect slots
    m_putReply = m_netManager->put(request, file);

    connect(m_putReply, &QNetworkReply::finished, this, &QXmppHttpUpload::uploadFinished);
    connect(m_putReply, &QNetworkReply::uploadProgress, this, &QXmppHttpUpload::uploadProgressed);
    connect(m_putReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &QXmppHttpUpload::uploadFailed);

    // delete file object after upload
    connect(m_putReply, &QNetworkReply::finished, file, &QFile::deleteLater);
    connect(m_putReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            file, &QFile::deleteLater);
}

void QXmppHttpUpload::abortUpload()
{
    if (m_putReply) {
        m_putReply->abort();
        emit uploadFailed(QNetworkReply::NoError);
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

/// Requests an upload slot and starts uploading the file
///
/// \param file The file to be uploaded.
/// \param allowParallel Enable to upload this file even while another file is uploading.
/// \param customFileName Changes the file name on the server.
/// \return Returns the upload's id. The id is unique per session (as long as the UploadManager
/// exists).

int QXmppUploadManager::uploadFile(const QFileInfo &file, bool allowParallel, QString customFileName)
{
    auto *upload = new QXmppHttpUpload(this);
    upload->setFileInfo(file);
    upload->setCustomFileName(customFileName);
    upload->setId(m_nextJobId++);

    // connect signals
    connect(upload, &QXmppHttpUpload::uploadFinished,
            this, &QXmppUploadManager::handleUploadFinished);
    connect(upload, &QXmppHttpUpload::uploadProgressed,
            this, &QXmppUploadManager::handleUploadProgressed);
    connect(upload, &QXmppHttpUpload::uploadFailed,
            this, &QXmppUploadManager::handleUploadFailed);

    m_uploads.append(upload);

    if (m_runningJobs == 0 || allowParallel)
        startNextUpload();

    return upload->id();
}

void QXmppUploadManager::startNextUpload()
{
    if (m_uploads.isEmpty())
        return;

    // If an upload is started in parallel, runningJobs will be larger than 0.
    // In this case we'll start the last added upload, otherwise the next in the queue.
    QXmppHttpUpload *upload = m_runningJobs > 0 ? m_uploads.last() : m_uploads.first();
    m_runningJobs++;

    QString reqId = requestUploadSlot(upload->fileInfo(), upload->customFileName());
    upload->setRequestId(reqId);

    if (reqId.isEmpty()) {
        m_runningJobs--;
        m_uploads.removeAll(upload);
        emit uploadFailed(upload);
        delete upload;

        // start next upload, if this wasn't a parallel upload
        if (m_runningJobs == 0)
            startNextUpload();
        return;
    }

    emit uploadStarted(upload);
}

/// Handles requested upload slots

void QXmppUploadManager::handleSlot(const QXmppHttpUploadSlotIq &slot)
{
    for (QXmppHttpUpload *upload : m_uploads) {
        if (upload->requestId() != slot.id())
            continue;

        if (!m_httpAllowed && !slot.hasHttpsUrls()) {
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
    for (QXmppHttpUpload *upload : m_uploads) {
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

/// Handles upload progress

void QXmppUploadManager::handleUploadProgressed(qint64 sent, qint64 total)
{
    qDebug() << "progress" << sent << total;
}

/// Handles finished uploads

void QXmppUploadManager::handleUploadFinished()
{
    m_runningJobs--;
    auto *upload = (QXmppHttpUpload*) sender();
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
