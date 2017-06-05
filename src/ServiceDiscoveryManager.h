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

//
// XEP-0030: Service Discovery | https://xmpp.org/extensions/xep-0030.html
//

#ifndef SERVICEDISCOVERYMANAGER_H
#define SERVICEDISCOVERYMANAGER_H

// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Elements/ErrorPayload.h>

class ServiceDiscoveryManager
{
public:
	ServiceDiscoveryManager();

	void setClient(Swift::Client*);
	void handleConnected();
	void handleServerDiscoInfoReceived(boost::shared_ptr<Swift::DiscoInfo> discoInfo,
		Swift::ErrorPayload::ref error);

private:
	Swift::Client* client;
};

#endif // SERVICEDISCOVERYMANAGER_H
