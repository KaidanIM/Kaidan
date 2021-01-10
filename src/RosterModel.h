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

// std
#include <optional>
// Qt
#include <QAbstractListModel>
#include <QVector>
// Kaidan
#include "RosterItem.h"

class Kaidan;
class RosterDb;
class MessageModel;
class Message;

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

	/**
	 * Retrieves the name of a roster item.
	 *
	 * @return Name of the roster item or a null string
	 */
	Q_INVOKABLE QString itemName(const QString &jid) const;

signals:
	void addItemRequested(const RosterItem &item);
	void removeItemRequested(const QString &jid);
	void updateItemRequested(const QString &jid,
	                         const std::function<void (RosterItem &)> &updateItem);
	void replaceItemsRequested(const QHash<QString, RosterItem> &items);

	/**
	 * Emitted, whan a subscription request was received
	 */
	void subscriptionRequestReceived(const QString &from, const QString &msg);

private slots:
	void handleItemsFetched(const QVector<RosterItem> &items);

	void addItem(const RosterItem &item);
	void removeItem(const QString &jid);
	void updateItem(const QString &jid,
	                const std::function<void (RosterItem &)> &updateItem);
	void replaceItems(const QHash<QString, RosterItem> &items);
	void handleMessageAdded(const Message &message);

private:
	/**
	 * Searches for the roster item with a given JID.
	 */
	std::optional<const RosterItem> findItem(const QString &jid) const;

	void insertContact(int i, const RosterItem &item);
	int updateItemPosition(int currentIndex);
	int positionToInsert(const RosterItem &item);

	RosterDb *m_rosterDb;
	QVector<RosterItem> m_items;
};
