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

#ifndef ROSTERMODEL_H
#define ROSTERMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include "RosterItem.h"

class Kaidan;
class RosterDb;
class MessageModel;

class RosterModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum RosterItemRoles {
		JidRole,
		NameRole,
		LastExchangedRole,
		UnreadMessagesRole,
		LastMessageRole,
	};

	RosterModel(RosterDb *rosterDb, QObject *parent = nullptr);

	void setMessageModel(MessageModel *model);

	Q_REQUIRED_RESULT bool isEmpty() const;
	Q_REQUIRED_RESULT int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_REQUIRED_RESULT QHash<int, QByteArray> roleNames() const override;
	Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role) const override;

signals:
	void addItemRequested(const RosterItem &item);
	void removeItemRequested(const QString &jid);
	void updateItemRequested(const QString &jid,
	                         const std::function<void (RosterItem &)> &updateItem);
	void replaceItemsRequested(const QHash<QString, RosterItem> &items);
	void setLastMessageRequested(const QString &contactJid, const QString &newLastMessage);
	void setLastExchangedRequested(const QString &contactJid, const QDateTime &newLastExchanged);

private slots:
	void handleItemsFetched(const QVector<RosterItem> &items);

	void addItem(const RosterItem &item);
	void removeItem(const QString &jid);
	void updateItem(const QString &jid,
	                const std::function<void (RosterItem &)> &updateItem);
	void replaceItems(const QHash<QString, RosterItem> &items);
	void setLastMessage(const QString &contactJid, const QString &newLastMessage);
	void setLastExchanged(const QString &contactJid, const QDateTime &newLastExchanged);

private:
	void insertContact(int i, const RosterItem &item);
	int updateItemPosition(int currentIndex);
	int positionToInsert(const RosterItem &item);

	RosterDb *rosterDb;
	QVector<RosterItem> m_items;
};

#endif // ROSTERMODEL_H
