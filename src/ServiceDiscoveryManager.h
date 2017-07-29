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

// gloox
#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/discohandler.h>

class ServiceDiscoveryManager : gloox::DiscoHandler
{
public:
	ServiceDiscoveryManager(gloox::Disco *disco);
	~ServiceDiscoveryManager();

	void setFeaturesAndIdentity();

	void handleDiscoInfo(const gloox::JID &from, const gloox::Disco::Info &info, int context);
	void handleDiscoItems(const gloox::JID &from, const gloox::Disco::Items &items, int context);
	void handleDiscoError(const gloox::JID &from, const gloox::Error *error, int context);
	bool handleDiscoSet(const gloox::IQ &iq);

private:
	gloox::Disco *disco;
};

#endif // SERVICEDISCOVERYMANAGER_H
