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

#include "RosterManager.h"
// Kaidan
#include "ClientWorker.h"
#include "Globals.h"
#include "Kaidan.h"
#include "VCardManager.h"
// QXmpp
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <QXmppUtils.h>

RosterManager::RosterManager(Kaidan *kaidan,
                             QXmppClient *client,
                             RosterModel *model,
                             AvatarFileStorage *avatarStorage,
                             VCardManager *vCardManager,
                             QObject *parent)
    : QObject(parent),
      kaidan(kaidan),
      client(client),
      model(model),
      avatarStorage(avatarStorage),
      vCardManager(vCardManager),
      manager(client->rosterManager())
{
	connect(&manager, &QXmppRosterManager::rosterReceived,
	        this, &RosterManager::populateRoster);

	connect(&manager, &QXmppRosterManager::itemAdded,
		this, [this, vCardManager, model] (const QString &jid) {
		emit model->addItemRequested(RosterItem(manager.getRosterEntry(jid)));

		vCardManager->fetchVCard(jid);
	});

	connect(&manager, &QXmppRosterManager::itemChanged,
		this, [this, model] (const QString &jid) {
		emit model->updateItemRequested(m_chatPartner, [=] (RosterItem &item) {
			item.setName(manager.getRosterEntry(jid).name());
		});
	});

	connect(&manager, &QXmppRosterManager::itemRemoved, model, &RosterModel::removeItemRequested);

	connect(&manager, &QXmppRosterManager::subscriptionReceived,
	        this, [kaidan] (const QString &jid) {
		// emit signal to ask user
		emit kaidan->subscriptionRequestReceived(jid, QString());
	});
	connect(kaidan, &Kaidan::subscriptionRequestAnswered,
	        this, [=] (QString jid, bool accepted) {
		if (accepted) {
			manager.acceptSubscription(jid);

			// do not send a subscription request if both users have already subscribed
			// each others presence
			if (manager.getRosterEntry(jid).subscriptionType() != QXmppRosterIq::Item::Both)
				manager.subscribe(jid);
		} else {
			manager.refuseSubscription(jid);
		}
	});

	// update local copy of chat partner
	connect(kaidan->getMessageModel(), &MessageModel::chatPartnerChanged,
	        this, [=] (const QString &jid) {
		        m_chatPartner = jid;
	        }
	);

	// user actions
	connect(kaidan, &Kaidan::addContact, this, &RosterManager::addContact);
	connect(kaidan, &Kaidan::removeContact, this, &RosterManager::removeContact);
	connect(kaidan, &Kaidan::renameContact, this, &RosterManager::renameContact);
	connect(kaidan, &Kaidan::sendMessage, this, &RosterManager::handleSendMessage);

	connect(client, &QXmppClient::messageReceived, this, &RosterManager::handleMessage);
}

void RosterManager::populateRoster()
{
	qDebug() << "[client] [RosterManager] Populating roster";
	// create a new list of contacts
	QHash<QString, RosterItem> items;
	const QStringList bareJids = manager.getRosterBareJids();
	const auto currentTime = QDateTime::currentDateTimeUtc();
	for (const auto &jid : bareJids) {
		items[jid] = RosterItem(manager.getRosterEntry(jid), currentTime);

		if (avatarStorage->getHashOfJid(jid).isEmpty())
			vCardManager->fetchVCard(jid);
	}

	// replace current contacts with new ones from server
	emit model->replaceItemsRequested(items);
}

void RosterManager::addContact(const QString &jid, const QString &name, const QString &msg)
{
	if (client->state() == QXmppClient::ConnectedState) {
		manager.addItem(jid, name);
		manager.subscribe(jid, msg);
	} else {
		emit kaidan->passiveNotificationRequested(
			tr("Could not add contact, as a result of not being connected.")
		);
		qWarning() << "[client] [RosterManager] Could not add contact, as a result of "
		              "not being connected.";
	}
}

void RosterManager::removeContact(const QString &jid)
{
	if (client->state() == QXmppClient::ConnectedState) {
		manager.unsubscribe(jid);
		manager.removeItem(jid);
	} else {
		emit kaidan->passiveNotificationRequested(
			tr("Could not remove contact, as a result of not being connected.")
		);
		qWarning() << "[client] [RosterManager] Could not remove contact, as a result of "
		              "not being connected.";
	}
}

void RosterManager::renameContact(const QString &jid, const QString &newContactName)
{
	if (client->state() == QXmppClient::ConnectedState) {
		manager.renameItem(jid, newContactName);
	} else {
		emit kaidan->passiveNotificationRequested(
			tr("Could not rename contact, as a result of not being connected.")
		);
		qWarning() << "[client] [RosterManager] Could not rename contact, as a result of "
		              "not being connected.";
	}
}

void RosterManager::handleSendMessage(const QString &jid, const QString &message,
                                      bool isSpoiler, const QString &spoilerHint)
{
	if (client->state() == QXmppClient::ConnectedState) {
		// update roster item
		const QString lastMessage =
		        isSpoiler ? spoilerHint.isEmpty() ? tr("Spoiler")
		                                          : spoilerHint
		                  : message;
		// sorting order in contact list
		const QDateTime dateTime = QDateTime::currentDateTimeUtc();

		emit model->updateItemRequested(jid,
		                                [=] (RosterItem &item) {
			item.setLastMessage(lastMessage);
			item.setLastExchanged(dateTime);
		});
	}
}

void RosterManager::handleMessage(const QXmppMessage &msg)
{
    if (msg.body().isEmpty() || msg.type() == QXmppMessage::Error)
		return;

	// msg.from() can be our JID, if it's a carbon/forward from another client
	QString fromJid = QXmppUtils::jidToBareJid(msg.from());
	bool sentByMe = fromJid == client->configuration().jidBare();
	QString contactJid = sentByMe ? QXmppUtils::jidToBareJid(msg.to())
	                              : fromJid;

	// update last exchanged datetime (sorting order in contact list)
	const QDateTime dateTime = QDateTime::currentDateTimeUtc();

	// update unread message counter, if chat is not active
	if (sentByMe) {
		// if we sent a message (with another device), reset counter
		emit model->updateItemRequested(contactJid,
		                                [dateTime] (RosterItem &item) {
			item.setLastExchanged(dateTime);
			item.setUnreadMessages(0);
		});
	} else if (m_chatPartner != contactJid) {
		emit model->updateItemRequested(contactJid,
		                                [dateTime] (RosterItem &item) {
			item.setLastExchanged(dateTime);
			item.setUnreadMessages(item.unreadMessages() + 1);
		});
	}
}
