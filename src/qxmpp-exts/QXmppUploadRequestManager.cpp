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

#include <QFileInfo>
#include <QMimeDatabase>

#include <QXmppClient.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppDiscoveryIq.h>
#include <QXmppDataForm.h>

#include "QXmppUploadRequestManager.h"

QString QXmppUploadService::jid() const
{
    return m_jid;
}

void QXmppUploadService::setJid(const QString &jid)
{
    m_jid = jid;
}

qint64 QXmppUploadService::sizeLimit() const
{
    return m_sizeLimit;
}

void QXmppUploadService::setSizeLimit(qint64 sizeLimit)
{
    m_sizeLimit = sizeLimit;
}

/// Requests an upload slot from the server. Better use \see uploadFile to also upload the file.
///
/// \param file The info of the file to be uploaded.
/// \param customFileName If not empty, this name is used instead of the file's name for requesting
/// the upload slot.
/// \return The id of the sent IQ. If sendPacket() isn't successful or there has no upload service
/// been found, an empty string will be returned.

QString QXmppUploadRequestManager::requestUploadSlot(const QFileInfo &file, QString customFileName)
{
    if (!serviceFound())
        return "";

    QMimeDatabase mimeDb;
    QXmppHttpUploadRequestIq iq;
    iq.setFrom(client()->configuration().jid());
    iq.setTo(m_uploadServices.first().jid());
    iq.setType(QXmppIq::Get);
    iq.setFileName(customFileName.isEmpty() ? file.fileName() : customFileName);
    iq.setSize(file.size());
    iq.setContentType(mimeDb.mimeTypeForFile(file));

    bool sent = client()->sendPacket(iq);

    if (sent)
        return iq.id();
    return "";
}

bool QXmppUploadRequestManager::serviceFound() const
{
    return !m_uploadServices.isEmpty();
}

bool QXmppUploadRequestManager::handleStanza(const QDomElement &element)
{
    if (QXmppHttpUploadSlotIq::isHttpUploadSlotIq(element)) {
        QXmppHttpUploadSlotIq slot;
        slot.parse(element);

        emit slotReceived(slot);
        return true;
    } else if (QXmppHttpUploadRequestIq::isHttpUploadRequestIq(element)) {
        QXmppHttpUploadRequestIq requestError;
        requestError.parse(element);

        emit requestFailed(requestError);
        return true;
    }
    return false;
}

void QXmppUploadRequestManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
    if (!iq.features().contains(ns_httpupload))
        return;

    for (const QXmppDiscoveryIq::Identity &identity : iq.identities()) {
        if (identity.category() == "store" && identity.type() == "file")
            goto addService;
    }
    return;

addService:
    QXmppUploadService service;
    service.setJid(iq.from());

    // get size limit
    bool isFormNsCorrect = false;
    for (const QXmppDataForm::Field &field : iq.form().fields()) {
        if (field.key() == "FORM_TYPE") {
            isFormNsCorrect = field.value() == ns_httpupload;
        } else if (isFormNsCorrect && field.key() == "max-file-size") {
            service.setSizeLimit(field.value().toULongLong());
            break;
        }
    }

    m_uploadServices.append(service);
    emit serviceFoundChanged();
}

void QXmppUploadRequestManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);
    // connect to service discovery manager
    QXmppDiscoveryManager *disco = client->findExtension<QXmppDiscoveryManager>();
    if (disco) {
        // scan info of all entities for upload services
        // Could this lead to another client being added as upload service?
        // Is that a vulnarability?
        connect(disco, &QXmppDiscoveryManager::infoReceived,
                this, &QXmppUploadRequestManager::handleDiscoInfo);

        // on client disconnect remove all upload services
        connect(client, &QXmppClient::disconnected, [this] () {
            m_uploadServices.clear();
            emit serviceFoundChanged();
        });
    }
}
