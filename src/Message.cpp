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

#include "Message.h"
#include <QMimeType>

MessageType Message::mediaTypeFromMimeType(const QMimeType &type)
{
	if (type.inherits("image/jpeg") || type.inherits("image/png") ||
	    type.inherits("image/gif"))
		return MessageType::MessageImage;
	if (type.inherits("audio/flac") || type.inherits("audio/mp4") ||
	         type.inherits("audio/ogg") || type.inherits("audio/wav") ||
	         type.inherits("audio/mpeg") || type.inherits("audio/webm"))
		return MessageType::MessageAudio;
	if (type.inherits("video/mpeg") || type.inherits("video/x-msvideo") ||
	         type.inherits("video/quicktime") || type.inherits("video/mp4") ||
	         type.inherits("video/x-matroska"))
		return MessageType::MessageVideo;
	if (type.inherits("text/plain"))
		return MessageType::MessageDocument;
	return MessageType::MessageFile;
}

bool Message::operator==(const Message &m) const
{
	return m.id() == id() &&
	       m.body() == body() &&
	       m.from() == from() &&
	       m.to() == to() &&
	       m.type() == type() &&
	       m.stamp() == stamp() &&
#if (QXMPP_VERSION) >= QT_VERSION_CHECK(1, 0, 0)
	       m.outOfBandUrl() == outOfBandUrl() &&
#endif
	       m.isSent() == isSent() &&
	       m.isDelivered() == isDelivered() &&
	       m.mediaType() == mediaType() &&
	       m.mediaContentType() == mediaContentType() &&
	       m.mediaLocation() == mediaLocation() &&
	       m.isEdited() == isEdited() &&
	       m.spoilerHint() == spoilerHint() &&
	       m.isSpoiler() == isSpoiler();
}

bool Message::operator!=(const Message &m) const
{
	return !operator==(m);
}

MessageType Message::mediaType() const
{
	return m_mediaType;
}

void Message::setMediaType(MessageType mediaType)
{
	m_mediaType = mediaType;
}

bool Message::sentByMe() const
{
	return m_sentByMe;
}

void Message::setSentByMe(bool sentByMe)
{
	m_sentByMe = sentByMe;
}

bool Message::isEdited() const
{
	return m_isEdited;
}

void Message::setIsEdited(bool isEdited)
{
	m_isEdited = isEdited;
}

bool Message::isSent() const
{
	return m_isSent;
}

void Message::setIsSent(bool isSent)
{
	m_isSent = isSent;
}

bool Message::isDelivered() const
{
	return m_isDelivered;
}

void Message::setIsDelivered(bool isDelivered)
{
	m_isDelivered = isDelivered;
}

QString Message::mediaLocation() const
{
	return m_mediaLocation;
}

void Message::setMediaLocation(const QString &mediaLocation)
{
	m_mediaLocation = mediaLocation;
}

QString Message::mediaContentType() const
{
	return m_mediaContentType;
}

void Message::setMediaContentType(const QString &mediaContentType)
{
	m_mediaContentType = mediaContentType;
}

QDateTime Message::mediaLastModified() const
{
	return m_mediaLastModified;
}

void Message::setMediaLastModified(const QDateTime &mediaLastModified)
{
	m_mediaLastModified = mediaLastModified;
}

qint64 Message::mediaSize() const
{
	return m_mediaSize;
}

void Message::setMediaSize(const qint64 &mediaSize)
{
	m_mediaSize = mediaSize;
}

bool Message::isSpoiler() const
{
	return m_isSpoiler;
}

void Message::setIsSpoiler(bool isSpoiler)
{
	m_isSpoiler = isSpoiler;
}

QString Message::spoilerHint() const
{
	return m_spoilerHint;
}

void Message::setSpoilerHint(const QString &spoilerHint)
{
	m_spoilerHint = spoilerHint;
}

#if (QXMPP_VERSION) < QT_VERSION_CHECK(1, 0, 0)
QString Message::outOfBandUrl() const
{
	return m_outOfBandUrl;
}

void Message::setOutOfBandUrl(const QString &outOfBandUrl)
{
	m_outOfBandUrl = outOfBandUrl;
}
#endif
