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

#include "RosterModel.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlTableModel>

static const char *rosterTableName = "Roster";

static void createTable()
{
	if (QSqlDatabase::database().tables().contains(QStringLiteral("Roster"))) {
		// The table already exists; we don't need to do anything.
		return;
	}

	QSqlQuery query;
	if (!query.exec(
		"CREATE TABLE IF NOT EXISTS 'Roster' ("
		"   'jid' TEXT NOT NULL,"
		"   'name' TEXT NOT NULL"
		")")) {
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
}

RosterModel::RosterModel(QObject *parent) : QSqlTableModel(parent)
{
	createTable();
	setTable(rosterTableName);
	setEditStrategy(QSqlTableModel::OnManualSubmit);

	select();
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
	for (int i = 0; i < rowCount(); ++i)
	{
		removeRow(i);
	}
}

void RosterModel::insertContact(QString jid_, QString name_)
{
	// create a new record
	QSqlRecord newRecord = record();

	// set the given data
	newRecord.setValue("jid", jid_);
	newRecord.setValue("name", name_);

	// inster the record into the DB (or print error)
	if (!insertRecord(rowCount(), newRecord)) {
		qWarning() << "Failed to save Contact into DB:"
			<< lastError().text();
	}

	submitAll();
}

void RosterModel::removeContactByJid(QString jid_)
{
	QSqlQuery newQuery;
	newQuery.exec(QString("DELETE FROM 'Roster' WHERE jid = '%1'").arg(jid_));
	submitAll();
}

void RosterModel::updateContactName(QString jid_, QString name_)
{
	QSqlQuery newQuery;
	newQuery.exec(QString("UPDATE 'Roster' SET name = '%1' WHERE jid = '%2'").arg(name_, jid_));
	submitAll();
}
