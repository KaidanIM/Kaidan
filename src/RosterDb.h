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

#pragma once

// Qt
#include <QObject>
class QSqlQuery;
class QSqlRecord;
// Kaidan
class RosterItem;
class Database;

class RosterDb : public QObject
{
	Q_OBJECT

public:
	RosterDb(Database *db, QObject *parent = nullptr);
	~RosterDb();

	static RosterDb *instance();

	static void parseItemsFromQuery(QSqlQuery &query, QVector<RosterItem> &items);

	/**
	 * Creates an @c QSqlRecord for updating an old item to a new item.
	 *
	 * @param oldMsg Full item as it is currently saved
	 * @param newMsg Full item as it should be after the update query ran.
	 */
	static QSqlRecord createUpdateRecord(const RosterItem &oldItem,
	                                     const RosterItem &newItem);

signals:
	void fetchItemsRequested(const QString &accountId);
	void itemsFetched(const QVector<RosterItem> &items);
	void updateItemRequested(const QString &jid,
	                         const std::function<void (RosterItem &)> &updateItem);
	void clearAllRequested();

public slots:
	void addItem(const RosterItem &item);
	void addItems(const QVector<RosterItem> &items);
	void removeItem(const QString &jid);
	void updateItem(const QString &jid,
	                const std::function<void (RosterItem &)> &updateItem);
	void replaceItems(const QHash<QString, RosterItem> &items);
	void setItemName(const QString &jid, const QString &name);
	void clearAll();

private slots:
	void fetchItems(const QString &accountId);

private:
	void updateItemByRecord(const QString &jid, const QSqlRecord &record);

	Database *m_db;

	static RosterDb *s_instance;
};
