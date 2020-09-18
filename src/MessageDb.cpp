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

#include "MessageDb.h"

// Qt
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
// Kaidan
#include "Globals.h"
#include "Utils.h"

MessageDb *MessageDb::s_instance = nullptr;

MessageDb::MessageDb(QObject *parent)
        : QObject(parent)
{
	Q_ASSERT(!MessageDb::s_instance);
	s_instance = this;

	connect(this, &MessageDb::fetchMessagesRequested,
	        this, &MessageDb::fetchMessages);

	connect(this, &MessageDb::fetchPendingMessagesRequested,
	        this, &MessageDb::fetchPendingMessages);
}

MessageDb::~MessageDb()
{
	s_instance = nullptr;
}

MessageDb *MessageDb::instance()
{
	return s_instance;
}

void MessageDb::parseMessagesFromQuery(QSqlQuery &query, QVector<Message> &msgs)
{
	// get indexes of attributes
	QSqlRecord rec = query.record();
	int idxFrom = rec.indexOf("author");
	int idxTo = rec.indexOf("recipient");
	int idxStamp = rec.indexOf("timestamp");
	int idxId = rec.indexOf("id");
	int idxBody = rec.indexOf("message");
	int idxDeliveryState = rec.indexOf("deliveryState");
	int idxMediaType = rec.indexOf("type");
	int idxOutOfBandUrl = rec.indexOf("mediaUrl");
	int idxMediaContentType = rec.indexOf("mediaContentType");
	int idxMediaLocation = rec.indexOf("mediaLocation");
	int idxMediaSize = rec.indexOf("mediaSize");
	int idxMediaLastModified = rec.indexOf("mediaLastModified");
	int idxIsEdited = rec.indexOf("edited");
	int idxSpoilerHint = rec.indexOf("spoilerHint");
	int idxIsSpoiler = rec.indexOf("isSpoiler");
	int idxErrorText = rec.indexOf("errorText");
	int idxReplaceId = rec.indexOf("replaceId");

	while (query.next()) {
		Message msg;
		msg.setFrom(query.value(idxFrom).toString());
		msg.setTo(query.value(idxTo).toString());
		msg.setStamp(QDateTime::fromString(
			query.value(idxStamp).toString(),
			Qt::ISODate
		));
		msg.setId(query.value(idxId).toString());
		msg.setBody(query.value(idxBody).toString());
		msg.setDeliveryState(static_cast<Enums::DeliveryState>(query.value(idxDeliveryState).toInt()));
		msg.setMediaType(static_cast<MessageType>(query.value(idxMediaType).toInt()));
		msg.setOutOfBandUrl(query.value(idxOutOfBandUrl).toString());
		msg.setMediaContentType(query.value(idxMediaContentType).toString());
		msg.setMediaLocation(query.value(idxMediaLocation).toString());
		msg.setMediaSize(query.value(idxMediaSize).toLongLong());
		msg.setMediaLastModified(QDateTime::fromMSecsSinceEpoch(
			query.value(idxMediaLastModified).toLongLong()
		));
		msg.setIsEdited(query.value(idxIsEdited).toBool());
		msg.setSpoilerHint(query.value(idxSpoilerHint).toString());
		msg.setErrorText(query.value(idxErrorText).toString());
		msg.setIsSpoiler(query.value(idxIsSpoiler).toBool());
		msg.setReplaceId(query.value(idxReplaceId).toString());
		msg.setReceiptRequested(true);	//this is useful with resending pending messages
		msgs << msg;
	}
}

QSqlRecord MessageDb::createUpdateRecord(const Message &oldMsg, const Message &newMsg)
{
	QSqlRecord rec;

	if (oldMsg.from() != newMsg.from())
		rec.append(Utils::createSqlField("author", newMsg.from()));
	if (oldMsg.to() != newMsg.to())
		rec.append(Utils::createSqlField("recipient", newMsg.to()));
	if (oldMsg.stamp() != newMsg.stamp())
		rec.append(Utils::createSqlField(
		        "timestamp",
		        newMsg.stamp().toString(Qt::ISODate)
		));
	if (oldMsg.id() != newMsg.id()) {
		// TODO: remove as soon as 'NOT NULL' was removed from id column
		if (newMsg.id().isEmpty())
			rec.append(Utils::createSqlField("id", QStringLiteral(" ")));
		else
			rec.append(Utils::createSqlField("id", newMsg.id()));
	}
	if (oldMsg.body() != newMsg.body())
		rec.append(Utils::createSqlField("message", newMsg.body()));
	if (oldMsg.deliveryState() != newMsg.deliveryState())
		rec.append(Utils::createSqlField("deliveryState", int(newMsg.deliveryState())));
	if (oldMsg.errorText() != newMsg.errorText())
		rec.append(Utils::createSqlField("errorText", newMsg.errorText()));
	if (oldMsg.mediaType() != newMsg.mediaType())
		rec.append(Utils::createSqlField("type", int(newMsg.mediaType())));
	if (oldMsg.outOfBandUrl() != newMsg.outOfBandUrl())
		rec.append(Utils::createSqlField("mediaUrl", newMsg.outOfBandUrl()));
	if (oldMsg.mediaContentType() != newMsg.mediaContentType())
		rec.append(Utils::createSqlField(
		        "mediaContentType",
		        newMsg.mediaContentType()
		));
	if (oldMsg.mediaLocation() != newMsg.mediaLocation())
		rec.append(Utils::createSqlField(
		         "mediaLocation",
		         newMsg.mediaLocation()
		));
	if (oldMsg.mediaSize() != newMsg.mediaSize())
		rec.append(Utils::createSqlField("mediaSize", newMsg.mediaSize()));
	if (oldMsg.mediaLastModified() != newMsg.mediaLastModified())
		rec.append(Utils::createSqlField(
			"mediaLastModified",
			newMsg.mediaLastModified().toMSecsSinceEpoch()
		));
	if (oldMsg.isEdited() != newMsg.isEdited())
		rec.append(Utils::createSqlField("edited", newMsg.isEdited()));
	if (oldMsg.spoilerHint() != newMsg.spoilerHint())
		rec.append(Utils::createSqlField("spoilerHint", newMsg.spoilerHint()));
	if (oldMsg.isSpoiler() != newMsg.isSpoiler())
		rec.append(Utils::createSqlField("isSpoiler", newMsg.isSpoiler()));
	if (oldMsg.replaceId() != newMsg.replaceId())
		rec.append(Utils::createSqlField("replaceId", newMsg.replaceId()));

	return rec;
}

void MessageDb::fetchMessages(const QString &user1, const QString &user2, int index)
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	query.setForwardOnly(true);

	QMap<QString, QVariant> bindValues;
	bindValues[":user1"] = user1;
	bindValues[":user2"] = user2;
	bindValues[":index"] = index;
	bindValues[":limit"] = DB_MSG_QUERY_LIMIT;

	Utils::execQuery(
		query,
		"SELECT * FROM " DB_TABLE_MESSAGES " "
		"WHERE (author = :user1 AND recipient = :user2) OR "
			"(author = :user2 AND recipient = :user1) "
		"ORDER BY timestamp DESC "
		"LIMIT :index, :limit",
		bindValues
	);

	QVector<Message> messages;
	parseMessagesFromQuery(query, messages);

	emit messagesFetched(messages);
}

Message MessageDb::fetchLastMessage(const QString &user1, const QString &user2)
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	query.setForwardOnly(true);

	QMap<QString, QVariant> bindValues = {
		{ QStringLiteral(":user1"), user1 },
		{ QStringLiteral(":user2"), user2 },
	};

	Utils::execQuery(
		query,
		"SELECT * FROM " DB_TABLE_MESSAGES " "
		"WHERE (author = :user1 AND recipient = :user2) OR "
		      "(author = :user2 AND recipient = :user1) "
		"ORDER BY timestamp DESC "
		"LIMIT 1",
		bindValues
	);

	QVector<Message> messages;
	parseMessagesFromQuery(query, messages);

	if (!messages.isEmpty())
		return messages.first();
	return {};
}

void MessageDb::addMessage(const Message &msg)
{
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);

	QSqlRecord record = db.record(DB_TABLE_MESSAGES);
	record.setValue("author", msg.from());
	record.setValue("recipient", msg.to());
	record.setValue("timestamp", msg.stamp().toString(Qt::ISODate));
	record.setValue("message", msg.body());
	record.setValue("id", msg.id().isEmpty() ? " " : msg.id());
	record.setValue("deliveryState", int(msg.deliveryState()));
	record.setValue("type", int(msg.mediaType()));
	record.setValue("edited", msg.isEdited());
	record.setValue("isSpoiler", msg.isSpoiler());
	record.setValue("spoilerHint", msg.spoilerHint());
	record.setValue("mediaUrl", msg.outOfBandUrl());
	record.setValue("mediaContentType", msg.mediaContentType());
	record.setValue("mediaLocation", msg.mediaLocation());
	record.setValue("mediaSize", msg.mediaSize());
	record.setValue("mediaLastModified", msg.mediaLastModified().toMSecsSinceEpoch());
	record.setValue("errorText", msg.errorText());
	record.setValue("replaceId", msg.replaceId());

	QSqlQuery query(db);
	Utils::execQuery(query, db.driver()->sqlStatement(
	        QSqlDriver::InsertStatement,
	        DB_TABLE_MESSAGES,
	        record,
	        false
	));
}

void MessageDb::removeMessage(const QString &id)
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	Utils::execQuery(
		query,
		"DELETE FROM " DB_TABLE_MESSAGES " WHERE id = ?",
		QVector<QVariant>() << id
	);
}

void MessageDb::removeAllMessages()
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	Utils::execQuery(query, "DELETE FROM " DB_TABLE_MESSAGES);
}

void MessageDb::updateMessage(const QString &id,
                              const std::function<void (Message &)> &updateMsg)
{
	// load current message item from db
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);

	QSqlQuery query(db);
	query.setForwardOnly(true);
	Utils::execQuery(
		query,
		"SELECT * FROM " DB_TABLE_MESSAGES " WHERE id = ? LIMIT 1",
		QVector<QVariant>() << id
	);

	QVector<Message> msgs;
	parseMessagesFromQuery(query, msgs);

	// update loaded item
	if (!msgs.isEmpty()) {
		Message msg = msgs.first();
		updateMsg(msg);

		// replace old message with updated one, if message has changed
		if (msgs.first() != msg) {
			// create an SQL record with only the differences
			QSqlRecord rec = createUpdateRecord(msgs.first(), msg);

			Utils::execQuery(
			        query,
			        db.driver()->sqlStatement(
			                QSqlDriver::UpdateStatement,
			                DB_TABLE_MESSAGES,
			                rec,
			                false
			        ) +
			        Utils::simpleWhereStatement(db.driver(), "id", id)
			);
		}
	}
}

void MessageDb::updateMessageRecord(const QString &id,
                                    const QSqlRecord &updateRecord)
{
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);
	QSqlQuery query(db);
	Utils::execQuery(
	        query,
	        db.driver()->sqlStatement(
	                QSqlDriver::UpdateStatement,
	                DB_TABLE_MESSAGES,
	                updateRecord,
	                false
	        ) +
	        Utils::simpleWhereStatement(db.driver(), "id", id)
	);
}

void MessageDb::fetchPendingMessages(const QString& userJid)
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	query.setForwardOnly(true);

	QMap<QString, QVariant> bindValues;
	bindValues[":user"] = userJid;
	bindValues[":deliveryState"] = int(Enums::DeliveryState::Pending);

	Utils::execQuery(
		query,
		"SELECT * FROM " DB_TABLE_MESSAGES " "
		"WHERE (author = :user AND deliveryState = :deliveryState) "
		"ORDER BY timestamp ASC",
		bindValues
	);

	QVector<Message> messages;
	parseMessagesFromQuery(query, messages);

	emit pendingMessagesFetched(messages);
}

