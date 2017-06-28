/*
 *  Kaidan - A user-friendly XMPP client for every device!
 * 
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 * 
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RosterManager.h"

RosterManager::RosterManager(RosterModel* rosterModel, gloox::Client *client)
{
	this->rosterModel = rosterModel;
	rosterManager = client->rosterManager();

	// register the roster updater as roster listener
	rosterUpdater = new RosterUpdater(rosterModel, rosterManager);
	rosterManager->registerRosterListener(rosterUpdater);
}

RosterManager::~RosterManager()
{
	rosterManager->removeRosterListener();
	delete rosterUpdater;
}

void RosterManager::addContact(const QString jid, const QString nick)
{
	// don't set any groups
	gloox::StringList groups;
	rosterManager->add(jid.toStdString(), nick.toStdString(), groups);
}

void RosterManager::removeContact(const QString jid)
{
	// cancel possible subscriptions
	rosterManager->cancel(jid.toStdString()); // don't send our presence anymore
	rosterManager->unsubscribe(jid.toStdString()); // don't receive the JID's presence anymore
	// remove contact from roster
	rosterManager->remove(jid.toStdString());
}

void RosterManager::setChatPartner(QString* jid)
{
	rosterUpdater->setChatPartner(jid);
}

void RosterManager::handleMessageSent(QString* jid, QString* message)
{
	rosterUpdater->handleMessageSent(jid, message);
}
