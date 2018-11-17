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

#ifndef QXMPPUPLOADMANAGER_H
#define QXMPPUPLOADMANAGER_H

#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include "QXmppUploadRequestManager.h"

class QNetworkAccessManager;
class QXmppHttpUploadRequestIq;
class QXmppUploadManager; // needed for QXmppHttpUpload

/// \class QXmppHttpUpload Represents a single HTTP file upload.

class QXMPP_EXPORT QXmppHttpUpload : public QXmppLoggable
{
    Q_OBJECT

public:
    QXmppHttpUpload(QXmppUploadManager *manager);
    QXmppHttpUpload(const QXmppHttpUpload &upload);
    ~QXmppHttpUpload();

    QXmppHttpUpload& operator=(const QXmppHttpUpload &upload);

    int id() const;
    void setId(int id);

    QString requestId() const;
    void setRequestId(const QString &requestId);

    QXmppHttpUploadRequestIq requestError() const;
    void setRequestError(const QXmppHttpUploadRequestIq &requestError);

    QFileInfo fileInfo() const;
    void setFileInfo(const QFileInfo &fileInfo);

    QString customFileName() const;
    void setCustomFileName(const QString &customFileName);

    QXmppHttpUploadSlotIq slot() const;
    void setSlot(const QXmppHttpUploadSlotIq &slot);

    qint64 progress() const;

public slots:
    void startUpload();
    void abortUpload();

signals:
    void uploadFinished();
    void uploadProgressed(qint64 sent, qint64 total);
    void uploadFailed(QNetworkReply::NetworkError code);

private slots:
    void handleProgressed(qint64 sent, qint64 total);

private:
    QXmppUploadManager *m_manager;

    QString m_customFileName;
    QFileInfo m_fileInfo;

    int m_id;
    QString m_requestId;

    QXmppHttpUploadRequestIq m_requestError;
    QXmppHttpUploadSlotIq m_slot;

    qint64 m_bytesSent;
    QNetworkReply::NetworkError m_uploadError;
    QNetworkAccessManager *m_netManager;
    QNetworkReply *m_putReply;
};

bool operator==(const QXmppHttpUpload &l, const QXmppHttpUpload &r);

/// \class QXmppHttpUploadManager This class extends the \see QXmppUploadCoreManager by also
/// handling the actual upload via. HTTP.

class QXMPP_EXPORT QXmppUploadManager : public QXmppUploadRequestManager
{
    Q_OBJECT

public:
    QXmppUploadManager();

    bool httpAllowed();
    void setHttpAllowed(bool httpAllowed);

public slots:
    int uploadFile(const QFileInfo &file, bool allowParallel = false, QString customFileName = "");

signals:
    void uploadStarted(const QXmppHttpUpload *upload);
    void uploadSucceeded(const QXmppHttpUpload *upload);
    void uploadFailed(const QXmppHttpUpload *upload);

private slots:
    void startNextUpload();

    void handleSlot(const QXmppHttpUploadSlotIq &slot);
    void handleRequestError(const QXmppHttpUploadRequestIq &request);

    void handleUploadFinished();
    void handleUploadProgressed(qint64 sent, qint64 total);
    void handleUploadFailed(QNetworkReply::NetworkError code);

private:
    bool m_httpAllowed = false;

    QList<QXmppHttpUpload*> m_uploads;
    int m_runningJobs = 0;
    int m_nextJobId = 0;
};

#endif // QXMPPUPLOADMANAGER_H
