/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

#include <QDomElement>
#include <QStringBuilder>

#include "MediaUtils.h"

static bool operator==(const QXmppStanza::Error &left, const QXmppStanza::Error &right) {
	return left.code() == right.code()
		&& left.text() == right.text()
		&& left.condition() == right.condition()
		&& left.type() == right.type();
}

static bool operator==(const QXmppElement &left, const QXmppElement &right) {
	return left.sourceDomElement() == right.sourceDomElement()
		&& left.attributeNames() == right.attributeNames()
		&& left.tagName() == right.tagName()
		&& left.value() == right.value();
}

static bool operator==(const QXmppExtendedAddress &left, const QXmppExtendedAddress &right) {
	return left.description() == right.description()
		&& left.jid() == right.jid()
		&& left.type() == right.type()
		&& left.isDelivered() == right.isDelivered();
}

static bool operator==(const QXmppStanza &left, const QXmppStanza &right) {
	return left.to() == right.to()
		&& left.from() == right.from()
		&& left.id() == right.id()
		&& left.lang() == right.lang()
		&& left.error() == right.error()
		&& left.extensions() == right.extensions()
		&& left.extendedAddresses() == right.extendedAddresses()
		&& left.isXmppStanza() == right.isXmppStanza();
}

static bool operator==(const QXmppMessage &left, const QXmppMessage &right) {
	return operator==(static_cast<const QXmppStanza &>(left), static_cast<const QXmppStanza &>(right))
		&& left.body() == right.body()
		&& left.isAttentionRequested() == right.isAttentionRequested()
		&& left.isReceiptRequested() == right.isReceiptRequested()
		&& left.mucInvitationJid() == right.mucInvitationJid()
		&& left.mucInvitationPassword() == right.mucInvitationPassword()
		&& left.mucInvitationReason() == right.mucInvitationReason()
		&& left.receiptId() == right.receiptId()
		&& left.stamp() == right.stamp()
		&& left.state() == right.state()
		&& left.subject() == right.subject()
		&& left.thread() == right.thread()
		&& left.type() == right.type()
		&& left.xhtml() == right.xhtml()
		&& left.isMarkable() == right.isMarkable()
		&& left.markedId() == right.markedId()
		&& left.markedThread() == right.markedThread()
		&& left.marker() == right.marker()
		&& left.isPrivate() == right.isPrivate()
		&& left.isXmppStanza() == right.isXmppStanza()
		&& left.outOfBandUrl() == right.outOfBandUrl()
		&& left.replaceId() == right.replaceId();
}

bool Message::operator==(const Message &m) const
{
	return ::operator==(static_cast<const QXmppMessage &>(m), static_cast<const QXmppMessage &>(*this))
		&& m.mediaType() == mediaType()
		&& m.isOwn() == isOwn()
		&& m.isEdited() == isEdited()
		&& m.deliveryState() == deliveryState()
		&& m.mediaLocation() == mediaLocation()
		&& m.mediaContentType() == mediaContentType()
		&& m.mediaLastModified() == mediaLastModified()
		&& m.mediaSize() == mediaSize()
		&& m.isSpoiler() == isSpoiler()
		&& m.spoilerHint() == spoilerHint()
		&& m.errorText() == errorText();
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

bool Message::isOwn() const
{
	return m_isOwn;
}

void Message::setIsOwn(bool isOwn)
{
	m_isOwn = isOwn;
}

bool Message::isEdited() const
{
	return m_isEdited;
}

void Message::setIsEdited(bool isEdited)
{
	m_isEdited = isEdited;
}

Enums::DeliveryState Message::deliveryState() const
{
	return m_deliveryState;
}

void Message::setDeliveryState(Enums::DeliveryState state)
{
	m_deliveryState = state;
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

QString Message::errorText() const
{
	return m_errorText;
}

void Message::setErrorText(const QString &errText)
{
	m_errorText = errText;
}

QString Message::previewText() const
{
	if (isSpoiler()) {
		if (spoilerHint().isEmpty()) {
			return tr("Spoiler");
		} else {
			return spoilerHint();
		}
	} else {
		if (mediaType() == Enums::MessageType::MessageText) {
			return body();
		} else {
			const auto text = MediaUtils::mediaTypeName(mediaType());

			if (!body().isEmpty())
				return text % QStringLiteral(": ") % body();
			return text;
		}
	}
}
