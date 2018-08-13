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

#include "RosterModel.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlDatabase>

RosterModel::RosterModel(QSqlDatabase *database, QObject *parent):
	QSqlTableModel(parent, *database)
{
	setTable("Roster");
	setEditStrategy(QSqlTableModel::OnManualSubmit);

	// sort from last time exchanged
	setSort(2, Qt::DescendingOrder);

	select();

	connect(this, &RosterModel::insertContactRequested, this, &RosterModel::insertContact);
	connect(this, &RosterModel::removeContactRequested, this, &RosterModel::removeContact);
	connect(this, &RosterModel::setContactNameRequested, this, &RosterModel::setContactName);
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
	for (int i = 0; i < rowCount(); ++i) {
		if (record(i).value("jid").toString() == jid) {
			removeRow(i);
			break;
		}
	}
	submitAll();
}

void RosterModel::setContactName(QString jid, QString name)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("jid").toString() == jid) {
			rec.setValue("name", name);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void RosterModel::setLastExchanged(const QString jid, QString date)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("jid").toString() == jid) {
			rec.setValue("lastExchanged", date);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void RosterModel::setUnreadMessageCount(const QString jid, const int count)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("jid").toString() == jid) {
			rec.setValue("unreadMessages", count);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void RosterModel::newUnreadMessage(const QString jid)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("jid").toString() == jid) {
			// increase unreadMessages by one
			rec.setValue("unreadMessages", rec.value("unreadMessages").toInt() + 1);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void RosterModel::setLastMessage(const QString jid, QString message)
{
	for (int i = 0; i < rowCount(); ++i) {
		QSqlRecord rec = record(i);
		if (rec.value("jid").toString() == jid) {
			rec.setValue("lastMessage", message);
			setRecord(i, rec);
			break;
		}
	}
	submitAll();
}

void RosterModel::replaceContacts(const ContactMap &contactList)
{
	// This will first remove a list of JIDs from the DB that were deleted on
	// the server, then it'll update all the nick names. This is made so
	// complicated, because otherwise information about lastExchanged, lastMessage,
	// etc. were lost.

	// list of the JIDs from the DB
	QStringList currentJids;
	for (int i = 0; i < rowCount(); ++i)
		currentJids << record(i).value("jid").toString();

	// add all JIDs to a delete list that are in the original list but not in the new one
	QList<int> rowsToDelete;
	for (int i = 0; i < currentJids.length(); i++) {
		if (!contactList.contains(currentJids.at(i)))
			rowsToDelete << i;
	}

	// remove rows
	for (const int row : rowsToDelete)
		removeRow(row);

	// Update all contact nicknames / add new contacts
	for (auto &jid : contactList.keys()) {
		QString name = contactList[jid];

		if (currentJids.contains(jid)) {
			// find row and set name
			for (int i = 0; i < rowCount(); ++i) {
				QSqlRecord rec = record(i);
				if (rec.value("jid").toString() == jid) {
					rec.setValue("name", name);
					setRecord(i, rec);
					break;
				}
			}
		} else {
			// add new row
			QSqlRecord rec = record();
			// set the given data
			rec.setValue("jid", jid);
			rec.setValue("name", name);
			rec.setValue("lastExchanged", QDateTime::currentDateTime().toUTC()
			                              .toString(Qt::ISODate));
			rec.setValue("unreadMessages", 0);

			if (!insertRecord(rowCount(), rec))
				qWarning() << "Failed to save Contact into DB:" << lastError().text();
		}
	}

	submitAll();
}
