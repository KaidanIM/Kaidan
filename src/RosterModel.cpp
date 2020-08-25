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

#include "RosterModel.h"
// Kaidan
#include "RosterDb.h"
#include "MessageModel.h"
#include "Kaidan.h"
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

	connect(Kaidan::instance(), &Kaidan::jidChanged, this, [=]() {
		beginResetModel();
		m_items.clear();
		endResetModel();

		emit rosterDb->fetchItemsRequested(Kaidan::instance()->jid());
	});
}

void RosterModel::setMessageModel(MessageModel *model)
{
	connect(model, &MessageModel::currentChatJidChanged, this, [=] (const QString &currentChatJid) {
		// reset unread message counter
		emit updateItemRequested(currentChatJid, [] (RosterItem &item) {
			item.setUnreadMessages(0);
		});
	});

	connect(model, &MessageModel::addMessageRequested,
	        this, &RosterModel::handleMessageAdded);
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

std::optional<const RosterItem> RosterModel::findItem(const QString &jid) const
{
	for (const auto &item : qAsConst(m_items)) {
		if (item.jid() == jid)
			return item;
	}

	return std::nullopt;
}

QString RosterModel::itemName(const QString &jid) const
{
	if (auto item = findItem(jid))
		return item->name();
	return {};
}

void RosterModel::handleItemsFetched(const QVector<RosterItem> &items)
{
	beginResetModel();
	m_items = items;
	std::sort(m_items.begin(), m_items.end());
	endResetModel();
}

void RosterModel::addItem(const RosterItem &item)
{
	insertContact(positionToInsert(item), item);
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

			m_items.replace(i, item);

			// item was changed: refresh all roles
			emit dataChanged(index(i), index(i), {});

			// check, if the position of the new item may be different
			updateItemPosition(i);
			return;
		}
	}
}

void RosterModel::replaceItems(const QHash<QString, RosterItem> &items)
{
	QVector<RosterItem> newItems;
	for (auto item : qAsConst(items)) {
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

void RosterModel::handleMessageAdded(const Message &message)
{
	const auto contactJid = message.sentByMe() ? message.to() : message.from();
	auto itr = std::find_if(m_items.begin(), m_items.end(), [&contactJid](const RosterItem &item) {
		return item.jid() == contactJid;
	});

	// contact not found
	if (itr == m_items.end())
		return;

	// new message is older than most recent event
	if (itr->lastExchanged() > message.stamp())
		return;

	QVector<int> changedRoles = {
		int(LastExchangedRole)
	};

	// last exchanged
	itr->setLastExchanged(message.stamp());

	// last message
	const auto lastMessage = message.previewText();
	if (itr->lastMessage() != lastMessage) {
		itr->setLastMessage(lastMessage);
		changedRoles << int(LastMessageRole);
	}

	// notify gui
	const auto i = std::distance(m_items.begin(), itr);
	const auto modelIndex = index(i);
	emit dataChanged(modelIndex, modelIndex, changedRoles);

	// move row to correct position
	updateItemPosition(i);
}

void RosterModel::insertContact(int i, const RosterItem &item)
{
	beginInsertRows(QModelIndex(), i, i);
	m_items.insert(i, item);
	endInsertRows();
}

int RosterModel::updateItemPosition(int currentPosition)
{
	const int newPosition = positionToInsert(m_items.at(currentPosition));
	if (currentPosition == newPosition)
		return currentPosition;

	beginMoveRows(QModelIndex(), currentPosition, currentPosition, QModelIndex(), newPosition);
	m_items.move(currentPosition, newPosition);
	endMoveRows();

	return newPosition;
}

int RosterModel::positionToInsert(const RosterItem &item)
{
	// prepend the item, if no timestamp is set
	if (item.lastExchanged().isNull())
		return 0;

	for (int i = 0; i < m_items.size(); i++) {
		if (item <= m_items.at(i))
			return i;
	}

	// append
	return m_items.size();
}
