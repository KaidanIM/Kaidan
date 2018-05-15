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
	QByteArray avatarBytes = QByteArray(vcard->photo().binval.c_str(),
	                                    vcard->photo().binval.length());

	if (!avatarBytes.isEmpty())
		avatarStorage->addAvatar(QString::fromStdString(jid.bare()), avatarBytes);
}

void VCardManager::handleVCardResult(VCardContext context, const gloox::JID &jid, gloox::StanzaError stanzaError)
{
}

void VCardManager::handlePresence(const gloox::Presence& presence)
{
	const gloox::VCardUpdate *vcupdate = presence.findExtension<gloox::VCardUpdate>
		(gloox::ExtVCardUpdate);

	// if their photo hash differs from what we have saved locally, refetch the vCard
	if (vcupdate && !vcupdate->hash().empty() &&
	    avatarStorage->getHashOfJid(QString::fromStdString(
	    presence.from().bare())).toStdString() != vcupdate->hash()) {
		fetchVCard(QString::fromStdString(presence.from().bare()));
	}
}

void VCardManager::onConnect()
{
	vCardManager->fetchVCard(client->jid().bare(), this);
}

void VCardManager::onDisconnect(gloox::ConnectionError error)
{
}

bool VCardManager::onTLSConnect(const gloox::CertInfo &info)
{
	return true;
}
