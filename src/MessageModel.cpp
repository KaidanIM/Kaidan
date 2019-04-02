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

// Kaidan
#include "MessageModel.h"
// C++
#include <iostream>
// Qt 5
#include <QMimeType>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

MessageModel::MessageModel(QSqlDatabase *database, QObject *parent):
	QSqlTableModel(parent, *database), database(database)
{
	setTable("Messages");
	// sort in descending order of the timestamp column
	setSort(4, Qt::DescendingOrder);

	setEditStrategy(QSqlTableModel::OnManualSubmit);

	connect(this, &MessageModel::chatPartnerChanged,
	        this, &MessageModel::applyRecipientFilter);
	connect(this, &MessageModel::addMessageRequested, this, &MessageModel::addMessage);
	connect(this, &MessageModel::setMessageAsSentRequested,
	        this, &MessageModel::setMessageAsSent);
	connect(this, &MessageModel::setMessageAsDeliveredRequested,
	        this, &MessageModel::setMessageAsDelivered);
	connect(this, &MessageModel::updateMessageRequested,
	        this, &MessageModel::updateMessage);
}

void MessageModel::applyRecipientFilter(QString recipient)
{
	const QString filterString = QString::fromLatin1(
		"(recipient = '%1' AND author = '%2') OR (recipient = '%2' AND author = '%1')")
		.arg(recipient, ownJid);

	setFilter(filterString);
	select();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
	if (role < Qt::UserRole)
		return QSqlTableModel::data(index, role);

	const QSqlRecord sqlRecord = record(index.row());
	return sqlRecord.value(role - Qt::UserRole);
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	// record() returns an empty QSqlRecord
	for (int i = 0; i < this->record().count(); i++) {
		roles.insert(Qt::UserRole + i, record().fieldName(i).toUtf8());
	}
	return roles;
}

MessageType MessageModel::messageTypeFromMimeType(const QMimeType &type)
{
	if (type.inherits("image/jpeg") || type.inherits("image/png") ||
	    type.inherits("image/gif"))
		return MessageType::MessageImage;
	else if (type.inherits("audio/flac") || type.inherits("audio/mp4") ||
	         type.inherits("audio/ogg") || type.inherits("audio/wav") ||
	         type.inherits("audio/mpeg") || type.inherits("audio/webm"))
		return MessageType::MessageAudio;
	else if (type.inherits("video/mpeg") || type.inherits("video/x-msvideo") ||
	         type.inherits("video/quicktime") || type.inherits("video/mp4") ||
	         type.inherits("video/x-matroska"))
		return MessageType::MessageVideo;
	else if (type.inherits("text/plain"))
		return MessageType::MessageDocument;
	return MessageType::MessageFile;
}

QString MessageModel::lastMessageId(QString jid) const
{
	return lastMsgIdCache.value(jid, "");
}

void MessageModel::setMessageAsSent(const QString msgId)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("id").toString() == msgId) {
			rec.setValue("isSent", true);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void MessageModel::setMessageAsDelivered(const QString msgId)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("id").toString() == msgId) {
			rec.setValue("isDelivered", true);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void MessageModel::updateMessage(const QString id, Message msg)
{
	QSqlRecord rec;
	int recId;
	bool found = false;
	for (int i = 0; i < rowCount(); ++i) {
		rec = record(i);
		if (rec.value("id").toString() == id) {
			recId = i;
			found = true;
			break;
		}
	}

	if (!found)
		return;

	QString newId = msg.id.isEmpty() ? id : msg.id;
	rec.setValue("id", newId);
	rec.setValue("edited", msg.edited);
	rec.setValue("isSent", msg.isSent);
	rec.setValue("isDelivered", msg.isDelivered);
	rec.setValue("isSpoiler", msg.isSpoiler);
	if (!msg.timestamp.isEmpty())
		rec.setValue("timestamp", msg.timestamp);
	if (!msg.message.isEmpty())
		rec.setValue("message", msg.message);
	if (!msg.mediaUrl.isEmpty())
		rec.setValue("mediaUrl", msg.mediaUrl);
	if (msg.mediaSize)
		rec.setValue("mediaSize", msg.mediaSize);
	if (!msg.mediaContentType.isEmpty())
		rec.setValue("mediaContentType", msg.mediaContentType);
	if (msg.mediaLastModified)
		rec.setValue("mediaLastModified", msg.mediaLastModified);
	if (!msg.mediaLocation.isEmpty())
		rec.setValue("mediaLocation", msg.mediaLocation);
	if (!msg.mediaThumb.isEmpty())
		rec.setValue("mediaThumb", msg.mediaThumb);
	if (!msg.mediaHashes.isEmpty())
		rec.setValue("mediaHashes", msg.mediaHashes);

	setRecord(recId, rec);
	submitAll();

	// check if we're author/recipient
	QVariant jid = rec.value("author").toString() == ownJid ? rec.value("recipient")
	               : rec.value("author");
	// update last message id
	lastMsgIdCache[jid.toString()] = newId;
}

void MessageModel::addMessage(Message msg)
{
	//
	// add the new message
	//

	QSqlRecord record = this->record();
	record.setValue("author", msg.author);
	record.setValue("recipient", msg.recipient);
	record.setValue("timestamp", msg.timestamp);
	record.setValue("message", msg.message);
	record.setValue("id", msg.id);
	record.setValue("isSent", msg.isSent);
	record.setValue("isDelivered", msg.isDelivered);
	record.setValue("type", (quint8) msg.type);
	record.setValue("edited", msg.edited);
	record.setValue("mediaUrl", msg.mediaUrl);
	record.setValue("isSpoiler", msg.isSpoiler);
	record.setValue("spoilerHint", msg.spoilerHint);
	if (msg.mediaSize)
		record.setValue("mediaSize", msg.mediaSize);
	record.setValue("mediaContentType", msg.mediaContentType);
	if (msg.mediaLastModified)
		record.setValue("mediaLastModified", msg.mediaLastModified);
	record.setValue("mediaLocation", msg.mediaLocation);
	record.setValue("mediaThumb", msg.mediaThumb);
	record.setValue("mediaHashes", msg.mediaHashes);

	if (!insertRecord(0, record)) {
		qWarning() << "Failed to add message to DB:" << lastError().text();
		return;
	}

	submitAll();

	// update last message id
	if (!msg.id.isEmpty()) {
		// check if we're author/recipient
		QString jid = msg.author == ownJid ? msg.recipient : msg.author;
		lastMsgIdCache[jid] = msg.id;
	}
}
