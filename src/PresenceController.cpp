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

#include "PresenceController.h"
#include <QObject>
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/Presence.h>
#include <boost/bind.hpp>

PresenceController::PresenceController(QObject* parent) : QObject(parent)
{

}

PresenceController::~PresenceController()
{

}

void PresenceController::setClient(Swift::Client* client_)
{
	client = client_;
	client->onPresenceReceived.connect(boost::bind(&PresenceController::handlePresenceReceived, this, _1));
}

void PresenceController::handlePresenceReceived(Swift::Presence::ref presence)
{
	// Automatically approve subscription requests
	if (presence->getType() == Swift::Presence::Subscribe)
	{
		Swift::Presence::ref response = Swift::Presence::create();
		response->setTo(presence->getFrom());
		response->setType(Swift::Presence::Subscribed);
		client->sendPresence(response);
	}
}
