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

#include "RosterModel.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>

static const char *rosterTableName = "Roster";

RosterModel::RosterModel(QSqlDatabase *database, QObject *parent):
	QSqlTableModel(parent, *database)
{
	setTable(rosterTableName);
	setEditStrategy(QSqlTableModel::OnManualSubmit);

	// sort from last time exchanged
	setSort(2, Qt::DescendingOrder);

	select();

	connect(this, &RosterModel::clearDataRequested, this, &RosterModel::clearData);
	connect(this, &RosterModel::insertContactRequested, this, &RosterModel::insertContact);
	connect(this, &RosterModel::removeContactRequested, this, &RosterModel::removeContact);
	connect(this, &RosterModel::editContactNameRequested, this, &RosterModel::editContactName);
	connect(this, &RosterModel::setLastExchangedRequested, this, &RosterModel::setLastExchanged);
	connect(this, &RosterModel::setUnreadMessageCountRequested, this, &RosterModel::setUnreadMessageCount);
	connect(this, &RosterModel::setLastMessageRequested, this, &RosterModel::setLastMessage);
	connect(this, &RosterModel::newUnreadMessageRequested, this, &RosterModel::newUnreadMessage);
	connect(this, &RosterModel::replaceContactsRequested, this, &RosterModel::replaceContacts);
}

QHash<int, QByteArray> RosterModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	// record() returns an empty QSqlRecord
	for (int i = 0; i < this->record().count(); i ++) {
		roles.insert(Qt::UserRole + i + 1, record().fieldName(i).toUtf8());
	}
	return roles;
}

QVariant RosterModel::data(const QModelIndex &index, int role) const
{
	QVariant value;

	if (index.isValid()) {
		if (role < Qt::UserRole) {
			value = QSqlQueryModel::data(index, role);
		} else {
			int columnIdx = role - Qt::UserRole - 1;
			QModelIndex modelIndex = this->index(index.row(), columnIdx);
			value = QSqlQueryModel::data(modelIndex, Qt::DisplayRole);
		}
	}
	return value;
}

void RosterModel::clearData()
{
	// remove all rows / contacts from the model
	for (int i = 0; i < rowCount(); ++i) {
		removeRow(i);
	}
}

void RosterModel::insertContact(QString jid, QString name)
{
	// create a new record
	QSqlRecord newRecord = record();

	// set the given data
	newRecord.setValue("jid", jid);
	newRecord.setValue("name", name);
	newRecord.setValue("lastExchanged", QDateTime::currentDateTime().toString(Qt::ISODate));
	newRecord.setValue("unreadMessages", 0);

	// inster the record into the DB (or print error)
	if (!insertRecord(rowCount(), newRecord)) {
		qWarning() << "Failed to save Contact into DB:"
		           << lastError().text();
	}

	submitAll();
}

void RosterModel::removeContact(QString jid)
{
	QSqlQuery query(database());
	if (!query.exec(QString("DELETE FROM 'Roster' WHERE jid = '%1'").arg(jid))) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	submit();
}

void RosterModel::editContactName(QString jid, QString name)
{
	QSqlQuery query(database());
	if (!query.exec(QString("UPDATE 'Roster' SET name = '%1' WHERE jid = '%2'").arg(name, jid))) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	submit();
}

QStringList RosterModel::getJidList()
{
	QStringList retVar;

	QSqlQuery query(database());
	if (!query.exec("SELECT jid FROM Roster")) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
	}

	int jidCol = query.record().indexOf("jid");

	// add all jids to the list
	while (query.next())
		retVar << query.value(jidCol).toString();
	return retVar;
}

void RosterModel::removeContactList(QStringList &jidList)
{
	QSqlQuery query(database());
	for (int i = 0; i < jidList.length(); i++) {
		if (!query.exec(QString("DELETE FROM 'Roster' WHERE jid = '%1'")
		    .arg(jidList.at(i)))) {
			qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
		}
	}
	submitAll();
}

void RosterModel::setLastExchanged(const QString jid, QString date)
{
	QSqlQuery newQuery(database());
	newQuery.exec(QString("UPDATE 'Roster' SET lastExchanged = '%1' WHERE jid = '%2'").arg(date, jid));
	submit();
}

int RosterModel::getUnreadMessageCount(const QString &jid)
{
	QSqlQuery query(database());

	query.prepare(QString("SELECT unreadMessages FROM Roster WHERE jid = '%1'").arg(jid));
	if (!query.exec()) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
		return 0;
	}

	query.next();
	return query.value("unreadMessages").toInt();
}

void RosterModel::setUnreadMessageCount(const QString jid, const int count)
{
	QSqlQuery query(database());
	query.prepare(QString("UPDATE Roster SET unreadMessages = %1 WHERE jid = '%2'")
	              .arg(QString::number(count), jid));

	if (!query.exec()) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
		qDebug() << query.lastQuery();
	}
	if (!select()) {
		qDebug() << "Error on select in RosterModel::setUnreadMessageCount";
	}
	submit();
}

void RosterModel::newUnreadMessage(const QString jid)
{
	int msgCount = getUnreadMessageCount(jid);
	msgCount++;
	setUnreadMessageCount(jid, msgCount);
}

void RosterModel::setLastMessage(const QString jid, QString message)
{
	QSqlQuery query(database());
	query.prepare(QString("UPDATE Roster SET lastMessage = '%1' WHERE jid = '%2'")
	              .arg(message, jid));

	if (!query.exec()) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
		qDebug() << query.lastQuery();
	}
	if (!select()) {
		qDebug() << "Error on select in RosterModel::setLastMessage";
	}
	submit();
}

void RosterModel::replaceContacts(const ContactMap &contactList)
{
	// This will first remove a list of JIDs from the DB that were deleted on
	// the server, then it'll update all the nick names. This is made so
	// complicated, because otherwise information about lastExchanged, lastMessage,
	// etc. will be lost.

	// list of the JIDs from the DB
	QStringList currentJids = getJidList();
	// a new list with all JIDs to delete
	QStringList jidsToDelete;

	// add all JIDs to the delete list that are in the original list
	// but not in the new from the server
	for (int i = 0; i < currentJids.length(); i++) {
		QString jidAtI = currentJids.at(i);
		if (!contactList.contains(jidAtI)) {
			jidsToDelete << jidAtI;
		}
	}

	// remove JIDs
	removeContactList(jidsToDelete);

	// Update all contact nicknames / add new contacts
	for (auto key : contactList.keys()) {
		QString jid = key;
		QString name = contactList[key];

		if (currentJids.contains(jid)) {
			editContactName(jid, name);
		} else {
			insertContact(jid, name);
		}
	}
}
