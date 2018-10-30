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

#include <QDomElement>
#include <QMimeDatabase>

#include "QXmppHttpUploadIq.h"

bool operator==(const QXmppHttpUploadRequestIq &l, const QXmppHttpUploadRequestIq &r)
{
    return l.fileName() == r.fileName() &&
           l.size() == r.size() &&
           l.contentType() == r.contentType();
}

QString QXmppHttpUploadRequestIq::fileName() const
{
    return m_fileName;
}

void QXmppHttpUploadRequestIq::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

qint64 QXmppHttpUploadRequestIq::size() const
{
    return m_size;
}

void QXmppHttpUploadRequestIq::setSize(const qint64 &size)
{
    m_size = size;
}

QMimeType QXmppHttpUploadRequestIq::contentType() const
{
    return m_contentType;
}

void QXmppHttpUploadRequestIq::setContentType(const QMimeType &type)
{
    m_contentType = type;
}

/// \cond
bool QXmppHttpUploadRequestIq::isHttpUploadRequestIq(const QDomElement &element)
{
    if (element.tagName() == "iq") {
        QDomElement request = element.firstChildElement("request");
        if (!request.isNull() && request.namespaceURI() == ns_httpupload)
            return true;
    }
    return false;
}

void QXmppHttpUploadRequestIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement request = element.firstChildElement("request");
    m_fileName = element.attribute("filename");
    m_size = element.attribute("size").toLongLong();
    if (element.hasAttribute("content-type")) {
        QMimeDatabase mimeDb;
        QMimeType type = mimeDb.mimeTypeForName(element.attribute("content-type"));
        if (!type.isDefault() && type.isValid())
            m_contentType = type;
    }
}

void QXmppHttpUploadRequestIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("request");
    writer->writeAttribute("xmlns", ns_httpupload);
    // filename and size are required
    writer->writeAttribute("filename", m_fileName);
    writer->writeAttribute("size", QString::number(m_size));
    // content-type is optional
    if (!m_contentType.isDefault() && m_contentType.isValid())
        writer->writeAttribute("content-type", m_contentType.name());
    writer->writeEndElement();
}
/// \endcond

bool operator==(const QXmppHttpUploadSlotIq &l, const QXmppHttpUploadSlotIq &r)
{
    return l.getUrl() == r.getUrl() &&
           l.putUrl() == r.putUrl() &&
           l.headerFields() == r.headerFields();
}

QUrl QXmppHttpUploadSlotIq::putUrl() const
{
    return m_putUrl;
}

void QXmppHttpUploadSlotIq::setPutUrl(const QUrl &putUrl)
{
    m_putUrl = putUrl;
}

QUrl QXmppHttpUploadSlotIq::getUrl() const
{
    return m_getUrl;
}

void QXmppHttpUploadSlotIq::setGetUrl(const QUrl &getUrl)
{
    m_getUrl = getUrl;
}

QMap<QString, QString> QXmppHttpUploadSlotIq::headerFields() const
{
    return m_headerFields;
}

void QXmppHttpUploadSlotIq::setHeaderFields(const QMap<QString, QString> &headerFields)
{
    m_headerFields.clear();
    for (QString &name : headerFields.keys()) {
        if (name == "Authorization" || name == "Cookie" || name == "Expires") {
            QString value = headerFields[name];
            m_headerFields[name] = value.replace("\n", "");
        }
    }
}

/// Returns whether the URLs (put and get) are both HTTPS URLs. It will return false, if the URLs
/// are only HTTP URLs or they're not set. HTTP URLs are not XEP-compliant.

bool QXmppHttpUploadSlotIq::hasHttpsUrls() const
{
    if (m_getUrl.toString().startsWith("https://") &&
        m_putUrl.toString().startsWith("https://"))
        return true;
    return false;
}

/// \cond
bool QXmppHttpUploadSlotIq::isHttpUploadSlotIq(const QDomElement &element)
{
    if (element.tagName() == "iq") {
        QDomElement slot = element.firstChildElement("slot");
        if (!slot.isNull() && slot.namespaceURI() == ns_httpupload)
            return true;
    }
    return false;
}

void QXmppHttpUploadSlotIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement slot = element.firstChildElement("slot");
    QDomElement put = slot.firstChildElement("put");
    m_getUrl = QUrl::fromEncoded(slot.firstChildElement("get").attribute("url").toUtf8());
    m_putUrl = QUrl::fromEncoded(put.attribute("url").toUtf8());
    if (put.hasChildNodes()) {
        QMap<QString, QString> headers;
        QDomNodeList headerNodes = put.childNodes();
        for (int i = 0; i < headerNodes.length(); i++) {
            QDomElement tag = headerNodes.at(i).toElement();
            if (tag.tagName() == "header" && tag.hasAttribute("name"))
                headers[tag.attribute("name")] = headerNodes.at(i).toCDATASection().data();
        }

        setHeaderFields(headers);
    }
}

void QXmppHttpUploadSlotIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("slot");
    writer->writeAttribute("xmlns", ns_httpupload);

    writer->writeStartElement("put");
    writer->writeAttribute("url", m_putUrl.toEncoded());
    if (!m_headerFields.isEmpty()) {
        for (const QString &name : m_headerFields.keys()) {
            writer->writeStartElement("header");
            writer->writeAttribute("name", name);
            writer->writeCDATA(m_headerFields[name]);
            writer->writeEndElement();
        }
    }
    writer->writeEndElement();

    writer->writeStartElement("get");
    writer->writeAttribute("url", m_getUrl.toEncoded());
    writer->writeEndElement();

    writer->writeEndElement();
}
/// \endcond
