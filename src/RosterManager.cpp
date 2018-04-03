/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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

RosterManager::RosterManager(Kaidan *kaidan, gloox::Client *client,
                             RosterModel* rosterModel, VCardManager *vCardManager,
                             QObject *parent)
	: QObject(parent), rosterModel(rosterModel)
{
	rosterManager = client->rosterManager();

	// register the roster updater as roster listener (asynchronous sub handling)
	rosterUpdater = new RosterUpdater(kaidan, rosterModel, rosterManager,
	                                  vCardManager);
	rosterManager->registerRosterListener(rosterUpdater, false);
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
	rosterManager->subscribe(jid.toStdString(), nick.toStdString(), groups);
}

void RosterManager::removeContact(const QString jid)
{
	// cancel possible subscriptions
	// don't send our presence anymore
	rosterManager->cancel(jid.toStdString());
	// don't receive the JID's presence anymore
	rosterManager->unsubscribe(jid.toStdString());
	// remove contact from roster
	rosterManager->remove(jid.toStdString());
}
