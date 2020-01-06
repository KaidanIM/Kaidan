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

#ifndef QXMPPHTTPUPLOADIQ_H
#define QXMPPHTTPUPLOADIQ_H

#include <QMap>
#include <QMimeType>
#include <QUrl>

#include <QXmppIq.h>

const QString ns_httpupload = "urn:xmpp:http:upload:0";

/// \brief Represents a request to the server for receiving an upload slot.
///
/// XEP-Version: 0.7
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppHttpUploadRequestIq : public QXmppIq
{
public:
    QString fileName() const;
    void setFileName(const QString &filename);

    qint64 size() const;
    void setSize(const qint64 &size);

    QMimeType contentType() const;
    void setContentType(const QMimeType &type);

    /// \cond
    static bool isHttpUploadRequestIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QString m_fileName;
    qint64 m_size;
    QMimeType m_contentType;
};

bool operator==(const QXmppHttpUploadRequestIq &l, const QXmppHttpUploadRequestIq &r);

/// \brief Represents a slot for uploading a file to using HTTP File Upload. The server will return
/// it after receiving a valid \see QXmppHttpUploadRequestIq.
///
/// XEP-Version: 0.7
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppHttpUploadSlotIq : public QXmppIq
{
public:
    QUrl putUrl() const;
    void setPutUrl(const QUrl &putUrl);

    QUrl getUrl() const;
    void setGetUrl(const QUrl &getUrl);

    QMap<QString, QString> headerFields() const;
    void setHeaderFields(const QMap<QString, QString> &headerFields);

    bool hasHttpsUrls() const;

    /// \cond
    static bool isHttpUploadSlotIq(const QDomElement &element);

protected:
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QUrl m_putUrl;
    QUrl m_getUrl;
    QMap<QString, QString> m_headerFields;
};

bool operator==(const QXmppHttpUploadSlotIq &l, const QXmppHttpUploadSlotIq &r);

#endif // QXMPPHTTPUPLOADIQ_H
