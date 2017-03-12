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

// Kaidan
#include "ServiceDiscoveryManager.h"
// Boost
#include <boost/bind.hpp>
// Swiften
#include <Swiften/Swiften.h>

ServiceDiscoveryManager::ServiceDiscoveryManager()
{

}

void ServiceDiscoveryManager::setClient(Swift::Client* client_)
{
	client = client_;
	client->onConnected.connect(boost::bind(&ServiceDiscoveryManager::handleConnected, this));
}

void ServiceDiscoveryManager::handleConnected()
{
	//
	// send own capabilities
	//

	Swift::DiscoInfo discoInfo;
	// add identity, TODO: Use phone for android/sfos builds
	discoInfo.addIdentity(Swift::DiscoInfo::Identity(APPLICATION_NAME, "client", "pc"));

	// XEP-0184: Message Delivery Receipts | http://xmpp.org/extensions/xep-0184.html
	discoInfo.addFeature(Swift::DiscoInfo::MessageDeliveryReceiptsFeature);

	client->getDiscoManager()->setDiscoInfo(discoInfo);


	//
	// request the service discovery info from server
	//

	Swift::GetDiscoInfoRequest::ref discoInfoRequest = Swift::GetDiscoInfoRequest::create(
		Swift::JID(client->getJID().getDomain()),
		client->getIQRouter()
	);

	discoInfoRequest->onResponse.connect(boost::bind(
		&ServiceDiscoveryManager::handleServerDiscoInfoReceived, this, _1, _2
	));
	discoInfoRequest->send();
}

void ServiceDiscoveryManager::handleServerDiscoInfoReceived(
	boost::shared_ptr<Swift::DiscoInfo> info, Swift::ErrorPayload::ref error)
{
	// TODO: check if the server supports our XEPs
}
