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

#ifndef QXMPPUPLOADREQUESTMANAGER_H
#define QXMPPUPLOADREQUESTMANAGER_H

#include <QXmppClientExtension.h>
#include "QXmppHttpUploadIq.h"

class QFileInfo;

class QXMPP_EXPORT QXmppUploadService
{
public:
    QString jid() const;
    void setJid(const QString &jid);

    qint64 sizeLimit() const;
    void setSizeLimit(qint64 sizeLimit);

private:
    QString m_jid;
    qint64 m_sizeLimit = -1;
};

/// \class QXmppUploadRequestManager This class implements the core of XEP-0369: HTTP File Upload.
/// It handles the discovery of \see QXmppUploadService and can send upload requests and outputs the
/// upload slots. It doesn't do the actual upload via. HTTP, \see QXmppUploadManager for that
/// purpose.

class QXMPP_EXPORT QXmppUploadRequestManager : public QXmppClientExtension
{
    Q_OBJECT
    Q_PROPERTY(bool serviceFound READ serviceFound NOTIFY serviceFoundChanged)

public:
    QString requestUploadSlot(const QFileInfo &file, QString customFileName = "");
    bool serviceFound() const;

signals:
    /// Emitted when an upload slot was received.
    void slotReceived(const QXmppHttpUploadSlotIq &slot);

    /// Emitted when the slot request failed.
    void requestFailed(const QXmppHttpUploadRequestIq &request);

    void serviceFoundChanged();

protected:
    void setClient(QXmppClient *client) override;
    bool handleStanza(const QDomElement &stanza) override;

private:
    void handleDiscoInfo(const QXmppDiscoveryIq &iq);

    QList<QXmppUploadService> m_uploadServices;
};

#endif // QXMPPUPLOADREQUESTMANAGER_H
