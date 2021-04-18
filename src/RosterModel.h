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
class MessageModel;
class Message;
enum class MessageOrigin : quint8;

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

	static RosterModel *instance();

	RosterModel(QObject *parent = nullptr);

	void setMessageModel(MessageModel *model);

	Q_REQUIRED_RESULT bool isEmpty() const;
	Q_REQUIRED_RESULT int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_REQUIRED_RESULT QHash<int, QByteArray> roleNames() const override;
	Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role) const override;

	/**
	 * Retrieves the name of a roster item or its JID's local part.
	 *
	 * @param accountJid JID of the account whose roster item is retrieved
	 * @param jid JID of the roster item
	 *
	 * @return the name of the roster item, or
	 * the local part of its JID if the found roster item has no name, or
	 * an empty string if no roster item with the given JID could be found
	 */
	Q_INVOKABLE QString itemName(const QString &accountJid, const QString &jid) const;

signals:
	void addItemRequested(const RosterItem &item);
	void removeItemRequested(const QString &jid);
	void updateItemRequested(const QString &jid,
	                         const std::function<void (RosterItem &)> &updateItem);
	void replaceItemsRequested(const QHash<QString, RosterItem> &items);

	/**
	 * Emitted to remove all roster items of an account or a specific roster item.
	 *
	 * @param accountJid JID of the account whose roster items are being removed
	 * @param jid JID of the roster item being removed (optional)
	 */
	void removeItemsRequested(const QString &accountJid, const QString &jid = {});

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

	/**
	 * Removes all roster items of an account or a specific roster item.
	 *
	 * @param accountJid JID of the account whose roster items are being removed
	 * @param jid JID of the roster item being removed (optional)
	 */
	void removeItems(const QString &accountJid, const QString &jid = {});

	void handleMessageAdded(const Message &message, MessageOrigin origin);

private:
	/**
	 * Searches for the roster item with a given JID.
	 */
	std::optional<const RosterItem> findItem(const QString &jid) const;

	void insertItem(int index, const RosterItem &item);
	int updateItemPosition(int currentIndex);
	int positionToInsert(const RosterItem &item);

	/**
	 * Determines a suitable roster item's name.
	 *
	 * @param jid JID of the roster item
	 * @param name name of the roster item which can be empty
	 *
	 * @return the passed item name if it is not empty, otherwise the local
	 * part of the passed JID
	 */
	QString determineItemName(const QString &jid, const QString &name) const;

	QVector<RosterItem> m_items;

	static RosterModel *s_instance;
};
