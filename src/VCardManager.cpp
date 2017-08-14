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

#include "VCardManager.h"
#include <gloox/vcardupdate.h>

VCardManager::VCardManager(gloox::Client *client, AvatarFileStorage *avatarStorage,
			   RosterModel *rosterModel)
{
	this->client = client;
	this->vCardManager = new gloox::VCardManager(client);
	this->avatarStorage = avatarStorage;
	this->rosterModel = rosterModel;
	client->registerPresenceHandler(this);
	client->registerConnectionListener(this);
}

VCardManager::~VCardManager()
{
	delete vCardManager;
}

void VCardManager::fetchVCard(QString jid)
{
	vCardManager->fetchVCard(gloox::JID(jid.toStdString()), this);
}

void VCardManager::handleVCard(const gloox::JID& jid, const gloox::VCard* vcard)
{
	std::string avatarString = vcard->photo().binval;
	if (avatarString.length() <= 0)
		return;

	QByteArray avatarBytes(avatarString.c_str(), avatarString.length());
	avatarStorage->addAvatar(QString::fromStdString(jid.bare()), avatarBytes);
}

void VCardManager::handleVCardResult(VCardContext context, const gloox::JID &jid, gloox::StanzaError stanzaError)
{
}

void VCardManager::handlePresence(const gloox::Presence& presence)
{
	const gloox::VCardUpdate *vcupdate = presence.findExtension<gloox::VCardUpdate>
		(gloox::ExtVCardUpdate);
	if (vcupdate && vcupdate->hash().size() > 0)
		fetchVCard(QString::fromStdString(presence.from().bare()));
}

void VCardManager::onConnect()
{
	vCardManager->fetchVCard(client->jid().bare(), this);
}

void VCardManager::onDisconnect(gloox::ConnectionError error)
{
}

bool VCardManager::onTLSConnect(const gloox::CertInfo& info)
{
	return true;
}
