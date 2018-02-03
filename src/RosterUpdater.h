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

#ifndef ROSTERUPDATER_H
#define ROSTERUPDATER_H

// Qt
#include <QObject>
#include <QSqlTableModel>
// gloox
#include <gloox/rosterlistener.h>
#include <gloox/rostermanager.h>
// Kaidan
#include "RosterModel.h"
#include "VCardManager.h"

class RosterUpdater : public gloox::RosterListener
{
public:
	RosterUpdater(RosterModel* rosterModel, gloox::RosterManager *rosterManager, VCardManager *vCardManager);
	~RosterUpdater();

	// gloox::RosterListener
	virtual void handleItemAdded(const gloox::JID& jid);
	virtual void handleItemSubscribed(const gloox::JID& jid);
	virtual void handleItemRemoved(const gloox::JID& jid);
	virtual void handleItemUpdated(const gloox::JID& jid);
	virtual void handleItemUnsubscribed(const gloox::JID& jid);
	virtual void handleRoster(const gloox::Roster& roster);
	virtual void handleRosterPresence(const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence::PresenceType presence, const std::string& msg);
	virtual void handleSelfPresence(const gloox::RosterItem& item, const std::string& resource,
		gloox::Presence::PresenceType presence, const std::string& msg);
	virtual bool handleSubscriptionRequest(const gloox::JID& jid, const std::string& msg);
	virtual bool handleUnsubscriptionRequest(const gloox::JID& jid, const std::string& msg);
	virtual void handleNonrosterPresence(const gloox::Presence& presence);
	virtual void handleRosterError(const gloox::IQ& iq);

private:
	RosterModel *rosterModel;
	gloox::RosterManager *rosterManager;
	VCardManager *vCardManager;
};

#endif // ROSTERUPDATER_H
