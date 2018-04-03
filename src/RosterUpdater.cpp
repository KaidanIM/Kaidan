/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

// Kaidan
#include "RosterUpdater.h"
#include "RosterModel.h"
// Std
#include <iostream>
#include <string.h>
// Qt 5
#include <QDateTime>
#include <QDebug>

RosterUpdater::RosterUpdater(RosterModel *rosterModel, gloox::RosterManager *rosterManager,
                             VCardManager *vCardManager, QObject *parent):
                             QObject(parent), rosterModel(rosterModel),
                             rosterManager(rosterManager), vCardManager(vCardManager)
{
}

RosterUpdater::~RosterUpdater()
{
}

void RosterUpdater::handleRoster(const gloox::Roster &roster)
{
	// create a new list of contacts
	ContactMap contactList;
	for (auto const& item : roster) {
		QString jid = QString::fromStdString(item.second->jidJID().bare());
		QString name = QString::fromStdString(item.second->name());

		contactList[jid] = name;
		vCardManager->fetchVCard(jid);
	}

	// replace current contacts with new ones from server
	emit rosterModel->replaceContactsRequested(contactList);
}

void RosterUpdater::handleItemAdded(const gloox::JID &jid)
{
	gloox::RosterItem *item = rosterManager->getRosterItem(jid);
	emit rosterModel->insertContactRequested(QString::fromStdString(jid.bare()),
	                                         QString::fromStdString(item->name()));
}

void RosterUpdater::handleItemRemoved(const gloox::JID &jid)
{
	emit rosterModel->removeContactRequested(QString::fromStdString(jid.bare()));
}

void RosterUpdater::handleItemUpdated(const gloox::JID &jid)
{
	gloox::RosterItem *item = rosterManager->getRosterItem(jid);
	emit rosterModel->editContactNameRequested(QString::fromStdString(jid.bare()),
	                                           QString::fromStdString(item->name()));
}

void RosterUpdater::handleRosterPresence(const gloox::RosterItem& item,
	const std::string& resource, gloox::Presence::PresenceType presence,
	const std::string& msg)
{
}

void RosterUpdater::handleSelfPresence(const gloox::RosterItem& item,
                                       const std::string& resource,
                                       gloox::Presence::PresenceType presence,
                                       const std::string& msg)
{
}

void RosterUpdater::handleItemSubscribed(const gloox::JID &jid)
{
}

void RosterUpdater::handleItemUnsubscribed(const gloox::JID &jid)
{
}

bool RosterUpdater::handleSubscriptionRequest(const gloox::JID& jid, const std::string& msg)
{
	qDebug() << "[RosterUpdater] Subscription request arrived from"
	         << QString::fromStdString(jid.full());

	// TODO: Ask user to accept

	// return value is ignored, because we're using asynchronous subscription handling
	return false;
}

bool RosterUpdater::handleUnsubscriptionRequest(const gloox::JID& jid,
	const std::string& msg)
{
	qDebug() << "[RosterUpdater]" << QString::fromStdString(jid.full())
	         << "has unsubscribed from your presence; also unsubscribing from its presence";

	rosterManager->unsubscribe(jid);

	// return value is ignored, because we're using asynchronous subscription handling
	return true;
}

void RosterUpdater::handleNonrosterPresence(const gloox::Presence& presence)
{
}

void RosterUpdater::handleRosterError(const gloox::IQ& iq)
{
	qWarning() << "[RosterUpdater] Error occured in IQ"
	           << QString::fromStdString(iq.id());
}
