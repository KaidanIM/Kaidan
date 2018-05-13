/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

static const char *conversationsTableName = "Messages";

MessageModel::MessageModel(QSqlDatabase *database, QObject *parent):
	QSqlTableModel(parent, *database), database(database)
{
	setTable(conversationsTableName);
	// sort in descending order of the timestamp column
	setSort(4, Qt::DescendingOrder);

	// Ensures that the model is sorted correctly after submitting a new row.
	setEditStrategy(QSqlTableModel::OnManualSubmit);

	connect(this, &MessageModel::chatPartnerChanged,
	        this, &MessageModel::applyRecipientFilter, Qt::BlockingQueuedConnection);
	connect(this, &MessageModel::ownJidChanged, [=](QString &ownJid) {
		this->ownJid = ownJid;
	});
	connect(this, &MessageModel::addMessageRequested, this, &MessageModel::addMessage);
	connect(this, &MessageModel::setMessageAsSentRequested, this, &MessageModel::setMessageAsSent);
	connect(this, &MessageModel::setMessageAsDeliveredRequested, this, &MessageModel::setMessageAsDelivered);
}

void MessageModel::applyRecipientFilter(QString recipient)
{
	const QString filterString = QString::fromLatin1(
		"(recipient = '%1' AND author = '%2') OR (recipient = '%2' AND "
		"author = '%1')").arg(recipient, ownJid);
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

void MessageModel::addMessage(const QString author, const QString recipient,
                              const QString timestamp, const QString message,
                              const QString msgId, bool sentByMe,
                              const QString author_resource, const QString recipient_resource)
{
	//
	// add the new message
	//

	QSqlRecord record = this->record();
	record.setValue("author", author);
	record.setValue("author_resource", author_resource);
	record.setValue("recipient", recipient);
	record.setValue("recipient_resource", recipient_resource);
	record.setValue("timestamp", timestamp);
	record.setValue("message", message);
	record.setValue("id", msgId);
	record.setValue("isSent", !sentByMe);
	record.setValue("isDelivered", !sentByMe);

	if (!insertRecord(rowCount(), record)) {
		qWarning() << "Failed to add message to DB:" << lastError().text();
		return;
	}

	submitAll();
}
