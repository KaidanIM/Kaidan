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

//
// XEP-0030: Service Discovery | https://xmpp.org/extensions/xep-0030.html
//

#include "ServiceDiscoveryManager.h"
// Std
#include <stdexcept>
// Qt
#include <QString>
#include <QDebug>
#include <QSysInfo>
// gloox
#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/carbons.h>
#include "gloox-extensions/httpuploadmanager.h"
// Kaidan
#include "Globals.h"

static const unsigned int DISCO_SERVER_ITEM_INFO = 10001;

ServiceDiscoveryManager::ServiceDiscoveryManager(gloox::Client *client, gloox::Disco *disco,
	gloox::HttpUploadManager *httpUploadManager) : client(client), disco(disco),
	httpUploadManager(httpUploadManager)
{
	// register as disco handler
	disco->registerDiscoHandler(this);
	// register as connection listener
	client->registerConnectionListener(this);

	setFeaturesAndIdentity();
}

ServiceDiscoveryManager::~ServiceDiscoveryManager()
{
}

void ServiceDiscoveryManager::onConnect()
{
	// request disco info & items from the server
	disco->getDiscoInfo(gloox::JID(client->server()), std::string(), this, 0);
	disco->getDiscoItems(gloox::JID(client->server()), std::string(), this, 0);
}

void ServiceDiscoveryManager::onDisconnect(gloox::ConnectionError error)
{
}

bool ServiceDiscoveryManager::onTLSConnect(const gloox::CertInfo &info)
{
	// accept; real check is made in ClientWorker
	return true;
}

void ServiceDiscoveryManager::setFeaturesAndIdentity()
{
	//
	// Announce our identity
	//

	// TODO: Annouce the client as phone on Android/SFOS/PM/...
	disco->setIdentity("client", "pc", APPLICATION_DISPLAY_NAME);

	// Set the software version
	std::string sysInfo = QSysInfo::prettyProductName().toStdString();
	disco->setVersion(APPLICATION_DISPLAY_NAME, VERSION_STRING, sysInfo);

	//
	// Announce features
	//

	// XEP-0184: Message Delivery Receipts | http://xmpp.org/extensions/xep-0184.html
	disco->addFeature(gloox::XMLNS_RECEIPTS);
}

void ServiceDiscoveryManager::handleDiscoInfo(const gloox::JID& from, const gloox::Disco::Info& info, int context)
{
	if (from.bare() == client->server()) {
		// XEP-0280: Message Carbons
		if (info.hasFeature(gloox::XMLNS_MESSAGE_CARBONS)) {
			// send IQ to server to enable message carbons
			gloox::IQ iq(gloox::IQ::Set, gloox::JID(), client->getID());
			iq.addExtension(new gloox::Carbons(gloox::Carbons::Enable));
			client->send(iq);
		}
	}

	if (context == DISCO_SERVER_ITEM_INFO) {
		for (gloox::Disco::Identity *identity : info.identities()) {
			// check identity for file storage
			if (identity->category() == "store" && identity->type() == "file") {
				httpUploadManager->tryAddFileStoreService(from, info);
				// break out of for loop to not add one service multiple times
				break;
			}
		}
	}
}

void ServiceDiscoveryManager::handleDiscoItems(const gloox::JID& from,
                                               const gloox::Disco::Items& items,
                                               int context)
{
	if (from.bare() == client->server()) {
		// get all disco items of the server and check their disco info
		for (gloox::Disco::Item *item : items.items()) {
			// don't rerequest info of server
			if (item->jid().bare() == client->server())
				continue;

			disco->getDiscoInfo(item->jid(), std::string(), this, DISCO_SERVER_ITEM_INFO);
		}
	}
}

bool ServiceDiscoveryManager::handleDiscoSet(const gloox::IQ& iq)
{
	// not handled disco set
	return false;
}

void ServiceDiscoveryManager::handleDiscoError(const gloox::JID &from,
                                               const gloox::Error *error,
                                               int context)
{
	qWarning() << "[client] [disco] Error occured with"
	           << QString::fromStdString(from.full()) << "in context" << context;
}

