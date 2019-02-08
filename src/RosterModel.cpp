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

#include "RosterModel.h"
// Kaidan
#include "RosterDb.h"
#include "MessageModel.h"
// C++
#include <functional>
// Qt
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

RosterModel::RosterModel(RosterDb *rosterDb, QObject *parent)
        : QAbstractListModel(parent),
          rosterDb(rosterDb)
{
	connect(rosterDb, &RosterDb::itemsFetched,
		this, &RosterModel::handleItemsFetched);

	connect(this, &RosterModel::addItemRequested, this, &RosterModel::addItem);
	connect(this, &RosterModel::addItemRequested, rosterDb, &RosterDb::addItem);

	connect(this, &RosterModel::removeItemRequested,
	        this, &RosterModel::removeItem);
	connect(this, &RosterModel::removeItemRequested,
	        rosterDb, &RosterDb::removeItem);

	connect(this, &RosterModel::updateItemRequested,
	        this, &RosterModel::updateItem);
	connect(this, &RosterModel::updateItemRequested,
	        rosterDb, &RosterDb::updateItem);

	connect(this, &RosterModel::replaceItemsRequested,
	        this, &RosterModel::replaceItems);
	connect(this, &RosterModel::replaceItemsRequested,
	        rosterDb, &RosterDb::replaceItems);

	emit rosterDb->fetchItemsRequested();
}

void RosterModel::setMessageModel(MessageModel *model)
{
	connect(model, &MessageModel::chatPartnerChanged,
	        this, [=] (const QString &chatPartner) {
		// reset unread message counter
		emit updateItemRequested(chatPartner,
		                         [] (RosterItem &item) {
			item.setUnreadMessages(0);
		});
	});
}

bool RosterModel::isEmpty() const
{
	return m_items.isEmpty();
}

int RosterModel::rowCount(const QModelIndex&) const
{
	return m_items.length();
}

QHash<int, QByteArray> RosterModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[JidRole] = "jid";
	roles[NameRole] = "name";
	roles[LastExchangedRole] = "lastExchanged";
	roles[UnreadMessagesRole] = "unreadMessages";
	roles[LastMessageRole] = "lastMessage";
	return roles;
}

QVariant RosterModel::data(const QModelIndex &index, int role) const
{
	if (!hasIndex(index.row(), index.column(), index.parent())) {
		qWarning() << "Could not get data from roster model." << index << role;
		return {};
	}

	switch (role) {
	case JidRole:
		return m_items.at(index.row()).jid();
	case NameRole:
		return m_items.at(index.row()).name();
	case LastExchangedRole:
		return m_items.at(index.row()).lastExchanged();
	case UnreadMessagesRole:
		return m_items.at(index.row()).unreadMessages();
	case LastMessageRole:
		return m_items.at(index.row()).lastMessage();
	}
	return {};
}

void RosterModel::handleItemsFetched(const QVector<RosterItem> &items)
{
	beginResetModel();
	m_items = items;
	endResetModel();
}

void RosterModel::addItem(const RosterItem &item)
{
	// prepend the item, if no timestamp is set
	if (item.lastExchanged().isNull()) {
		insertContact(0, item);
		return;
	}

	// index where to add the new contact
	int i = 0;
	for (const auto &itrItem : m_items) {
		if (item.lastExchanged().toMSecsSinceEpoch() >= itrItem.lastExchanged().toMSecsSinceEpoch()) {
			insertContact(i, item);
			return;
		}
		i++;
	}

	// append the item to the end of the list
	insertContact(i, item);
}

void RosterModel::removeItem(const QString &jid)
{
	QMutableVectorIterator<RosterItem> itr(m_items);
	int i = 0;
	while (itr.hasNext()) {
		if (itr.next().jid() == jid) {
			beginRemoveRows(QModelIndex(), i, i);
			itr.remove();
			endRemoveRows();
			return;
		}
		i++;
	}

}

void RosterModel::updateItem(const QString &jid,
                             const std::function<void (RosterItem &)> &updateItem)
{
	for (int i = 0; i < m_items.length(); i++) {
		if (m_items.at(i).jid() == jid) {
			// update item
			RosterItem item = m_items.at(i);
			updateItem(item);

			// check if item was actually modified
			if (m_items.at(i) == item)
				return;

			// check, if the position of the new item may be different
			if (item.lastExchanged() == m_items.at(i).lastExchanged()) {
				beginRemoveRows(QModelIndex(), i, i);
				m_items.removeAt(i);
				endRemoveRows();

				// add the item at the same position
				insertContact(i, item);
			} else {
				beginRemoveRows(QModelIndex(), i, i);
				m_items.removeAt(i);
				endRemoveRows();

				// put to new position
				addItem(item);
			}
			break;
		}
	}
}

void RosterModel::replaceItems(const QHash<QString, RosterItem> &items)
{
	QVector<RosterItem> newItems;
	for (auto item : items.values()) {
		// find old item
		auto oldItem = std::find_if(
			m_items.begin(),
			m_items.end(),
			[&] (const RosterItem &oldItem) {
				return oldItem.jid() == item.jid();
			}
		);

		// use the old item's values, if found
		if (oldItem != m_items.end()) {
			item.setLastMessage(oldItem->lastMessage());
			item.setLastExchanged(oldItem->lastExchanged());
			item.setUnreadMessages(oldItem->unreadMessages());
		}

		newItems << item;
	}

	// replace all items
	handleItemsFetched(newItems);
}

void RosterModel::insertContact(int i, const RosterItem &item)
{
	beginInsertRows(QModelIndex(), i, i);
	m_items.insert(i, item);
	endInsertRows();
}
