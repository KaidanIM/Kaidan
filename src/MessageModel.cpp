/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

// Kaidan
#include "MessageModel.h"
// C++
#include <iostream>
// Qt 5
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

static const char *conversationsTableName = "Messages";

MessageModel::MessageModel(QSqlDatabase *database, QObject *parent) : QSqlTableModel(parent, *database)
{
	this->database = database;
	setTable(conversationsTableName);
	// sort in descending order of the timestamp column
	setSort(4, Qt::DescendingOrder);

	// Ensures that the model is sorted correctly after submitting a new row.
	setEditStrategy(QSqlTableModel::OnManualSubmit);
}

void MessageModel::applyRecipientFilter(QString *recipient_, QString *author_)
{
	const QString filterString = QString::fromLatin1("(recipient = '%1' AND "
	                             "author = '%2') OR (recipient = '%2' AND author = '%1')").arg(
	                                     *recipient_, *author_);
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

void MessageModel::setMessageAsSent(const QString msgId)
{
	QSqlQuery newQuery(*database);
	newQuery.exec(QString("UPDATE 'Messages' SET 'isSent' = 1 WHERE id = '%1'").arg(msgId));
	submitAll();
}

void MessageModel::setMessageAsDelivered(const QString msgId)
{
	QSqlQuery newQuery(*database);
	newQuery.exec(QString("UPDATE 'Messages' SET 'isDelivered' = 1 WHERE id = '%1'").arg(msgId));
	submitAll();
}

void MessageModel::addMessage(const QString* author, const QString* author_resource,
                              const QString* recipient, const QString* recipient_resource,
                              const QString* timestamp, const QString* message, const QString* msgId,
                              bool sentByMe)
{
	//
	// add the new message
	//

	QSqlRecord record = this->record();
	record.setValue("author", *author);
	record.setValue("author_resource", *author_resource);
	record.setValue("recipient", *recipient);
	record.setValue("recipient_resource", *recipient_resource);
	record.setValue("timestamp", *timestamp);
	record.setValue("message", *message);
	record.setValue("id", *msgId);
	record.setValue("isSent", sentByMe ? false : true);
	record.setValue("isDelivered", sentByMe ? false : true);
	record.setValue("isRead", sentByMe ? false : true);
	record.setValue("isReadByMe", sentByMe ? true : false);

	if (!insertRecord(rowCount(), record)) {
		qWarning() << "Failed to add message to DB:" << lastError().text();
		return;
	}

	submitAll();
}
