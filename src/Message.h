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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QXmppMessage.h>
#include "Enums.h"
class QMimeType;

using namespace Enums;

/**
 * @brief This class is used to load messages from the database and use them in
 * the @c MessageModel. The class inherits from @c QXmppMessage and most
 * properties are shared.
 */
class Message : public QXmppMessage
{
public:
	static MessageType mediaTypeFromMimeType(const QMimeType&);

	/**
	 * Compares another @c Message with this. Only attributes that are saved in the
	 * database are checked.
	 */
	bool operator==(const Message &m) const;
	bool operator!=(const Message &m) const;

	MessageType mediaType() const;
	void setMediaType(MessageType mediaType);

	bool sentByMe() const;
	void setSentByMe(bool sentByMe);

	bool isEdited() const;
	void setIsEdited(bool isEdited);

	bool isSent() const;
	void setIsSent(bool isSent);

	bool isDelivered() const;
	void setIsDelivered(bool isDelivered);

	QString mediaLocation() const;
	void setMediaLocation(const QString &mediaLocation);

	QString mediaContentType() const;
	void setMediaContentType(const QString &mediaContentType);

	QDateTime mediaLastModified() const;
	void setMediaLastModified(const QDateTime &mediaLastModified);

	qint64 mediaSize() const;
	void setMediaSize(const qint64 &mediaSize);

	bool isSpoiler() const;
	void setIsSpoiler(bool isSpoiler);

	QString spoilerHint() const;
	void setSpoilerHint(const QString &spoilerHint);

#if (QXMPP_VERSION) < QT_VERSION_CHECK(1, 0, 0)
	QString outOfBandUrl() const;
	void setOutOfBandUrl(const QString &outOfBandUrl);
#endif

private:
	/**
	 * Media type of the message, e.g. a text or image.
	 */
	MessageType m_mediaType = MessageType::MessageText;

	/**
	 * True if the message was sent by the user.
	 */
	bool m_sentByMe = true;

	/**
	 * True if the orginal message was edited.
	 */
	bool m_isEdited = false;

	/**
	 * True if the message was sent.
	 */
	bool m_isSent = false;

	/**
	 * True if a sent message was delivered to the contact.
	 */
	bool m_isDelivered = false;

	/**
	 * Location of the media on the local storage.
	 */
	QString m_mediaLocation;

	/**
	 * Media content type, e.g. "image/jpeg".
	 */
	QString m_mediaContentType;

	/**
	 * Size of the file in bytes.
	 */
	qint64 m_mediaSize = 0;

	/**
	 * Timestamp of the last modification date of the file locally on disk.
	 */
	QDateTime m_mediaLastModified;

	/**
	 * True if the message is a spoiler message.
	 */
	bool m_isSpoiler = false;

	/**
	 * Hint of the spoiler message.
	 */
	QString m_spoilerHint;

#if (QXMPP_VERSION) < QT_VERSION_CHECK(1, 0, 0)
	QString m_outOfBandUrl;
#endif
};

#endif // MESSAGE_H
