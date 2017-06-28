/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
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

#ifndef ROSTERUPDATER_H
#define ROSTERUPDATER_H

// Qt
#include <QObject>
#include <QSqlTableModel>
// gloox
#include <gloox/message.h>
#include <gloox/messagehandler.h>
#include <gloox/rosterlistener.h>
#include <gloox/rostermanager.h>
// Kaidan
#include "RosterModel.h"

class RosterUpdater : public gloox::RosterListener, gloox::MessageHandler
{
public:
	RosterUpdater(RosterModel* rosterModel, gloox::RosterManager *rosterManager);
	~RosterUpdater();
	void setChatPartner(QString *jid);

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
	
	// gloox::MessageHandler
	virtual void handleMessage(const gloox::Message &msg, gloox::MessageSession *session = 0);
	
	void handleMessageSent(QString *jid, QString *message);
	void updateLastExchangedOfJid(QString *jid);
	void newUnreadMessageForJid(QString *jid);
	void resetUnreadMessagesForJid(QString *jid);

private:
	RosterModel *rosterModel;
	QString *chatPartner;
	gloox::RosterManager *rosterManager;
};

#endif // ROSTERUPDATER_H
