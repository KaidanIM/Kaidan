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

#ifndef VCARDMANAGER_H
#define VCARDMANAGER_H

// gloox
#include <gloox/client.h>
#include <gloox/vcardhandler.h>
#include <gloox/vcardmanager.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
// Kaidan
#include "AvatarFileStorage.h"
#include "RosterModel.h"

class VCardManager : public gloox::VCardHandler, public gloox::PresenceHandler,
	public gloox::ConnectionListener
{
public:
	VCardManager(gloox::Client *client, AvatarFileStorage *avatarStorage, RosterModel *rosterModel);
	~VCardManager();
	
	void fetchVCard(QString jid);
	virtual void handleVCard(const gloox::JID &jid, const gloox::VCard *vcard);
	virtual void handleVCardResult(VCardContext context, const gloox::JID &jid,
				       gloox::StanzaError stanzaError = gloox::StanzaErrorUndefined);
	virtual void handlePresence(const gloox::Presence &presence);
	virtual void onConnect();
	virtual void onDisconnect(gloox::ConnectionError error);
	virtual bool onTLSConnect(const gloox::CertInfo &info);

private:
	gloox::Client *client;
	gloox::VCardManager *vCardManager;
	AvatarFileStorage *avatarStorage;
	RosterModel *rosterModel;
};

#endif // VCARDMANAGER_H
