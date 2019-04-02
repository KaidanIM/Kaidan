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
#include "Kaidan.h"
#include "Globals.h"
#include "VCardManager.h"
#include "ClientWorker.h"
// QXmpp
#include <QXmppClient.h>
#include <QXmppUtils.h>
#include <QXmppRosterManager.h>

RosterManager::RosterManager(Kaidan *kaidan, QXmppClient *client, RosterModel *model,
	                       AvatarFileStorage *avatarStorage, VCardManager *vCardManager,
	                       QObject *parent)
	: QObject(parent), kaidan(kaidan), client(client), model(model),
	avatarStorage(avatarStorage), vCardManager(vCardManager), manager(client->rosterManager())
{
	connect(&manager, &QXmppRosterManager::rosterReceived,
	        this, &RosterManager::populateRoster);

	connect(&manager, &QXmppRosterManager::itemAdded, [=] (QString jid) {
		QXmppRosterIq::Item item = manager.getRosterEntry(jid);
		emit model->insertContactRequested(jid, item.name());

		vCardManager->fetchVCard(jid);
	});

	connect(&manager, &QXmppRosterManager::itemChanged, [=] (QString jid) {
		QXmppRosterIq::Item item = manager.getRosterEntry(jid);
		emit model->setContactNameRequested(jid, item.name());
	});

	connect(&manager, &QXmppRosterManager::itemRemoved, [=] (QString jid) {
		emit model->removeContactRequested(jid);
	});

	connect(&manager, &QXmppRosterManager::subscriptionReceived, [=] (QString jid) {
		// emit signal to ask user
		emit kaidan->subscriptionRequestReceived(jid, QString());
	});
	connect(kaidan, &Kaidan::subscriptionRequestAnswered, [=] (QString jid, bool accepted) {
		if (accepted)
			manager.acceptSubscription(jid);
		else
			manager.refuseSubscription(jid);
	});

	// user actions
	connect(kaidan, &Kaidan::addContact, this, &RosterManager::addContact);
	connect(kaidan, &Kaidan::removeContact, this, &RosterManager::removeContact);
	connect(kaidan, &Kaidan::sendMessage, this, &RosterManager::handleSendMessage);

	connect(kaidan, &Kaidan::chatPartnerChanged, [=] (QString chatPartner) {
		this->chatPartner = chatPartner;

		// reset unread message counter
		emit model->setUnreadMessageCountRequested(chatPartner, 0);
	});

	connect(client, &QXmppClient::messageReceived, this, &RosterManager::handleMessage);
}

void RosterManager::populateRoster()
{
	qDebug() << "[client] [RosterManager] Populating roster";
	// create a new list of contacts
	ContactMap contactList;
	for (auto const& jid : manager.getRosterBareJids()) {
		QString name = manager.getRosterEntry(jid).name();
		contactList[jid] = name;

		if (avatarStorage->getHashOfJid(jid).isEmpty())
			vCardManager->fetchVCard(jid);
	}

	// replace current contacts with new ones from server
	emit model->replaceContactsRequested(contactList);
}

void RosterManager::addContact(const QString jid, const QString name, const QString msg)
{
	if (client->state() == QXmppClient::ConnectedState) {
		manager.subscribe(jid, msg);
		manager.renameItem(jid, name);
	} else {
		emit kaidan->passiveNotificationRequested(
			tr("Could not add contact, as a result of not being connected.")
		);
		qWarning() << "[client] [RosterManager] Could not add contact, as a result of "
		              "not being connected.";
	}
}

void RosterManager::removeContact(const QString jid)
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

void RosterManager::handleSendMessage(const QString jid, const QString message)
{
	if (client->state() == QXmppClient::ConnectedState) {
		// update last message of the contact
		emit model->setLastMessageRequested(jid, message);

		// update last exchanged datetime (sorting order in contact list)
		QString dateTime = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
		emit model->setLastExchangedRequested(jid, dateTime);
	}
}

void RosterManager::handleMessage(const QXmppMessage &msg)
{
	if (msg.body().isEmpty())
		return;

	// TODO: Check if it's a carbon message (will need QXmpp v0.10)
	// msg.from() can be our JID, if it's a carbon/forward from another client
	QString fromJid = QXmppUtils::jidToBareJid(msg.from());
	bool sentByMe = fromJid == client->configuration().jidBare();
	QString contactJid = sentByMe ? QXmppUtils::jidToBareJid(msg.to())
	                              : fromJid;

	// update last message of the contact
	emit model->setLastMessageRequested(contactJid, msg.body());

	// update last exchanged datetime (sorting order in contact list)
	QString dateTime = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
	emit model->setLastExchangedRequested(contactJid, dateTime);

	// when we sent a message we can ignore all unread message notifications
	if (sentByMe)
		emit model->setUnreadMessageCountRequested(contactJid, 0);
	// update unread message counter, if chat is not active
	else if (chatPartner != contactJid)
		emit model->newUnreadMessageRequested(contactJid);
}
