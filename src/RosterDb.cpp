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

#include "RosterDb.h"
// Kaidan
#include "Database.h"
#include "Globals.h"
#include "Utils.h"
// Qt
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

RosterDb::RosterDb(Database *db, QObject *parent)
        : QObject(parent),
          m_db(db)
{
	connect(this, &RosterDb::fetchItemsRequested, this, &RosterDb::fetchItems);
}

void RosterDb::parseItemsFromQuery(QSqlQuery &query, QVector<RosterItem> &items)
{
	QSqlRecord rec = query.record();
	int idxJid = rec.indexOf("jid");
	int idxName = rec.indexOf("name");
	int idxLastExchanged = rec.indexOf("lastExchanged");
	int idxUnreadMessages = rec.indexOf("unreadMessages");
	int idxLastMessage = rec.indexOf("lastMessage");

	while (query.next()) {
		RosterItem item;
		item.setJid(query.value(idxJid).toString());
		item.setName(query.value(idxName).toString());
		item.setLastExchanged(QDateTime::fromString(
			query.value(idxLastExchanged).toString(),
			Qt::ISODateWithMs
		));
		item.setUnreadMessages(query.value(idxUnreadMessages).toInt());
		item.setLastMessage(query.value(idxLastMessage).toString());

		items << item;
	}
}

QSqlRecord RosterDb::createUpdateRecord(const RosterItem &oldItem, const RosterItem &newItem)
{
	QSqlRecord rec;
	if (oldItem.jid() != newItem.jid())
		rec.append(Utils::createSqlField("jid", newItem.jid()));
	if (oldItem.name() != newItem.name())
		rec.append(Utils::createSqlField("name", oldItem.name()));
	if (oldItem.lastMessage() != newItem.lastMessage())
		rec.append(Utils::createSqlField("lastMessage", newItem.lastMessage()));
	if (oldItem.lastExchanged() != newItem.lastExchanged())
		rec.append(Utils::createSqlField(
			"lastExchanged",
			newItem.lastExchanged().toString(Qt::ISODateWithMs)
		));
	if (oldItem.unreadMessages() != newItem.unreadMessages())
		rec.append(Utils::createSqlField(
			"unreadMessages",
			newItem.unreadMessages()
		));
	return rec;
}

void RosterDb::addItem(const RosterItem &item)
{
	addItems(QVector<RosterItem>() << item);
}

void RosterDb::addItems(const QVector<RosterItem> &items)
{
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);
	m_db->transaction();

	QSqlQuery query(db);
	Utils::prepareQuery(query, db.driver()->sqlStatement(
		QSqlDriver::InsertStatement,
		DB_TABLE_ROSTER,
		db.record(DB_TABLE_ROSTER),
		true
	));

	for (const auto &item : items) {
		query.addBindValue(item.jid());
		query.addBindValue(item.name());
		query.addBindValue(item.lastExchanged().toString(Qt::ISODateWithMs));
		query.addBindValue(item.unreadMessages());
		query.addBindValue(item.lastMessage());
		Utils::execQuery(query);
	}

	m_db->commit();
}

void RosterDb::removeItem(const QString &jid)
{
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);
	QSqlQuery query(db);
	Utils::execQuery(
	        query,
	        "DELETE FROM Roster WHERE jid = ?",
	        QVector<QVariant>() << jid
	);
}

void RosterDb::updateItem(const QString &jid,
			  const std::function<void (RosterItem &)> &updateItem)
{
	// load current roster item from db
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);

	QSqlQuery query(db);
	query.setForwardOnly(true);

	Utils::execQuery(
	        query,
	        "SELECT * FROM Roster WHERE jid = ? LIMIT 1",
	        QVector<QVariant>() << jid
	);

	QVector<RosterItem> items;
	parseItemsFromQuery(query, items);

	// update loaded item
	if (!items.isEmpty()) {
		RosterItem item = items.first();
		updateItem(item);

		// replace old item with updated one, if item has changed
		if (items.first() != item) {
			// create an SQL record with only the differences
			QSqlRecord rec = createUpdateRecord(items.first(), item);

			Utils::execQuery(
			        query,
			        db.driver()->sqlStatement(
			                QSqlDriver::UpdateStatement,
			                DB_TABLE_ROSTER,
			                rec,
			                false
			        ) +
			        Utils::simpleWhereStatement(db.driver(), "jid", jid)
			);
		}
	}
}

void RosterDb::replaceItems(const QHash<QString, RosterItem> &items)
{
	// load current items
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);
	QSqlQuery query(db);
	query.setForwardOnly(true);
	Utils::execQuery(query, "SELECT * FROM Roster");

	QVector<RosterItem> currentItems;
	parseItemsFromQuery(query, currentItems);

	m_db->transaction();

	QSet<QString> newJids = items.keys().toSet();

	for (const auto &oldItem : qAsConst(currentItems)) {
		// We will remove the already existing JIDs, so we get a set of JIDs that
		// are completely new.
		//
		// By calling remove(), we also find out whether the JID is already
		// existing or not.
		if (newJids.remove(oldItem.jid())) {
			// item is also included in newJids -> update

			// name is (currently) the only attribute that is defined by the
			// XMPP roster and so could cause a change
			if (oldItem.name() != items[oldItem.jid()].name())
				setItemName(oldItem.jid(), items[oldItem.jid()].name());
		} else {
			// item is not included in newJids -> delete
			removeItem(oldItem.jid());
		}
	}

	// now add the completely new JIDs
	for (const QString &jid : newJids)
		addItem(items[jid]);

	m_db->commit();
}

void RosterDb::setItemName(const QString &jid, const QString &name)
{
	QSqlDatabase db = QSqlDatabase::database(DB_CONNECTION);
	QSqlQuery query(db);

	QSqlRecord rec;
	rec.append(Utils::createSqlField("name", name));

	Utils::execQuery(
		query,
		db.driver()->sqlStatement(
			QSqlDriver::UpdateStatement,
			DB_TABLE_ROSTER,
			rec,
			false
		) +
		Utils::simpleWhereStatement(db.driver(), "jid", jid)
	);
}

void RosterDb::clearAll()
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	Utils::execQuery(query, "DELETE FROM Roster");
}

void RosterDb::fetchItems()
{
	QSqlQuery query(QSqlDatabase::database(DB_CONNECTION));
	query.setForwardOnly(true);
	Utils::execQuery(query, "SELECT * FROM Roster");

	QVector<RosterItem> items;
	parseItemsFromQuery(query, items);

	emit itemsFetched(items);
}
