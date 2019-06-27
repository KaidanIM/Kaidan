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

#include "MessageModel.h"
// Kaidan
#include "Kaidan.h"
#include "MessageDb.h"
// Qt 5
#include <QMimeType>
// QXmpp
#include <QXmppUtils.h>

MessageModel::MessageModel(Kaidan *kaidan, MessageDb *msgDb, QObject *parent)
        : QAbstractListModel(parent),
          kaidan(kaidan),
          msgDb(msgDb)
{
	connect(msgDb, &MessageDb::messagesFetched,
	        this, &MessageModel::handleMessagesFetched);

	connect(this, &MessageModel::addMessageRequested,
	        this, &MessageModel::addMessage);
	connect(this, &MessageModel::addMessageRequested,
	        msgDb, &MessageDb::addMessage);

	connect(this, &MessageModel::updateMessageRequested,
	        this, &MessageModel::updateMessage);
	connect(this, &MessageModel::updateMessageRequested,
	        msgDb, &MessageDb::updateMessage);

	connect(this, &MessageModel::setMessageAsSentRequested,
	        this, &MessageModel::setMessageAsSent);
	connect(this, &MessageModel::setMessageAsSentRequested,
	        msgDb, &MessageDb::setMessageAsSent);

	connect(this, &MessageModel::setMessageAsDeliveredRequested,
	        this, &MessageModel::setMessageAsDelivered);
	connect(this, &MessageModel::setMessageAsDeliveredRequested,
	        msgDb, &MessageDb::setMessageAsDelivered);
}

MessageModel::~MessageModel() = default;

bool MessageModel::isEmpty() const
{
	return m_messages.isEmpty();
}

int MessageModel::rowCount(const QModelIndex&) const
{
	return m_messages.length();
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Timestamp] = "timestamp";
	roles[Id] = "id";
	roles[Sender] = "sender";
	roles[Recipient] = "recipient";
	roles[Body] = "body";
	roles[SentByMe] = "sentByMe";
	roles[MediaType] = "mediaType";
	roles[IsEdited] = "isEdited";
	roles[IsSent] = "isSent";
	roles[IsDelivered] = "isDelivered";
	roles[MediaUrl] = "mediaUrl";
	roles[MediaSize] = "mediaSize";
	roles[MediaContentType] = "mediaContentType";
	roles[MediaLastModified] = "mediaLastModifed";
	roles[MediaLocation] = "mediaLocation";
	roles[MediaThumb] = "mediaThumb";
	roles[IsSpoiler] = "isSpoiler";
	roles[SpoilerHint] = "spoilerHint";
	return roles;
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
	if (!hasIndex(index.row(), index.column(), index.parent())) {
		qWarning() << "Could not get data from message model." << index << role;
		return {};
	}
	Message msg = m_messages.at(index.row());

	switch (role) {
	case Timestamp:
		return msg.stamp();
	case Id:
		return msg.id();
	case Sender:
		return msg.from();
	case Recipient:
		return msg.to();
	case Body:
		return msg.body();
	case SentByMe:
		return msg.sentByMe();
	case MediaType:
		return int(msg.mediaType());
	case IsEdited:
		return msg.isEdited();
	case IsSent:
		return msg.isSent();
	case IsDelivered:
		return msg.isDelivered();
	case MediaUrl:
		return msg.outOfBandUrl();
	case MediaLocation:
		return msg.mediaLocation();
	case MediaContentType:
		return msg.mediaContentType();
	case MediaSize:
		return msg.mediaLastModified();
	case MediaLastModified:
		return msg.mediaLastModified();
	case IsSpoiler:
		return msg.isSpoiler();
	case SpoilerHint:
		return msg.spoilerHint();

	// TODO: add (only useful as soon as we have got SIMS)
	case MediaThumb:
		return {};
	}
	return {};
}

void MessageModel::fetchMore(const QModelIndex &)
{
	emit msgDb->fetchMessagesRequested(kaidan->getJid(), chatPartner(),
	                                   m_messages.size());
}

bool MessageModel::canFetchMore(const QModelIndex &) const
{
	return !m_fetchedAll;
}

QString MessageModel::chatPartner()
{
	return m_chatPartner;
}

void MessageModel::setChatPartner(const QString &chatPartner)
{
	if (chatPartner == m_chatPartner)
		return;

	m_chatPartner = chatPartner;
	m_fetchedAll = false;

	emit chatPartnerChanged(chatPartner);
	clearAll();
}

bool MessageModel::canCorrectMessage(const QString &msgId) const
{
	// Only allow correction of the latest message sent by us
	for (const auto &msg : m_messages) {
		if (msg.from() == kaidan->getJid())
			return msg.id() == msgId;
	}
	return false;
}

void MessageModel::handleMessagesFetched(const QVector<Message> &msgs)
{
	if (msgs.isEmpty())
		return;

	beginInsertRows(QModelIndex(), rowCount(), rowCount() + msgs.length() - 1);
	for (auto msg : msgs) {
		msg.setSentByMe(kaidan->getJid() == msg.from());
		m_messages << msg;
	}
	endInsertRows();

	if (msgs.length() < DB_MSG_QUERY_LIMIT)
		m_fetchedAll = true;
}

void MessageModel::clearAll()
{
	if (!m_messages.isEmpty()) {
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
		m_messages.clear();
		endRemoveRows();
	}
}

void MessageModel::insertMessage(int idx, const Message &msg)
{
	beginInsertRows(QModelIndex(), idx, idx);
	m_messages.insert(idx, msg);
	endInsertRows();
}

void MessageModel::addMessage(const Message &msg)
{
	if (QXmppUtils::jidToBareJid(msg.from()) == m_chatPartner
	    || QXmppUtils::jidToBareJid(msg.to()) == m_chatPartner) {
		// index where to add the new message
		int i = 0;
		for (const auto &message : m_messages) {
			if (msg.stamp() > message.stamp()) {
				insertMessage(i, msg);
				return;
			}
			i++;
		}

		// add message to the end of the list
		insertMessage(i, msg);
	}
}

void MessageModel::updateMessage(const QString &id,
                                 const std::function<void (Message &)> &updateMsg)
{
	for (int i = 0; i < m_messages.length(); i++) {
		if (m_messages.at(i).id() == id) {
			// update message
			Message msg = m_messages.at(i);
			updateMsg(msg);

			// check if item was actually modified
			if (m_messages.at(i) == msg)
				return;

			// check, if the position of the new message may be different
			if (msg.stamp() == m_messages.at(i).stamp()) {
				beginRemoveRows(QModelIndex(), i, i);
				m_messages.removeAt(i);
				endRemoveRows();

				// add the message at the same position
				insertMessage(i, msg);
			} else {
				beginRemoveRows(QModelIndex(), i, i);
				m_messages.removeAt(i);
				endRemoveRows();

				// put to new position
				addMessage(msg);
			}
			break;
		}
	}
}

void MessageModel::setMessageAsSent(const QString &msgId)
{
	updateMessage(msgId, [] (Message &msg) {
		msg.setIsSent(true);
	});
}

void MessageModel::setMessageAsDelivered(const QString &msgId)
{
	updateMessage(msgId, [] (Message &msg) {
		msg.setIsDelivered(true);
	});
}
