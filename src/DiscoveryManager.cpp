/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

#include "DiscoveryManager.h"
// QXmpp
#include <QXmppDiscoveryManager.h>
#include <QXmppDiscoveryIq.h>

DiscoveryManager::DiscoveryManager(QXmppClient *client, QObject *parent)
	: QObject(parent), m_client(client), m_manager(client->findExtension<QXmppDiscoveryManager>())
{
	// we're a normal client (not a server, gateway, server component, etc.)
	m_manager->setClientCategory("client");
	m_manager->setClientName(APPLICATION_DISPLAY_NAME);
#if defined Q_OS_ANDROID || defined UBUNTU_TOUCH
	// on Ubuntu Touch and Android we're always a mobile client
	m_manager->setClientType("phone");
#else
	// Plasma Mobile packages won't differ from desktop builds, so we need to check the mobile
	// variable on runtime.
	if (!qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_MOBILE"))
		m_manager->setClientType("phone");
	else
		m_manager->setClientType("pc");
#endif

	connect(client, &QXmppClient::connected, this, &DiscoveryManager::handleConnection);
	connect(m_manager, &QXmppDiscoveryManager::infoReceived,
	        this, &DiscoveryManager::handleInfo);
	connect(m_manager, &QXmppDiscoveryManager::itemsReceived,
	        this, &DiscoveryManager::handleItems);
}

DiscoveryManager::~DiscoveryManager()
{
}

void DiscoveryManager::handleConnection()
{
	// request disco info & items from the server
	m_manager->requestInfo(m_client->configuration().domain());
	m_manager->requestItems(m_client->configuration().domain());
}

void DiscoveryManager::handleInfo(const QXmppDiscoveryIq&)
{
	// TODO: enable carbons and discovery http file upload
}

void DiscoveryManager::handleItems(const QXmppDiscoveryIq &iq)
{
	// request info from all items
	const QList<QXmppDiscoveryIq::Item> items = iq.items();
	for (const QXmppDiscoveryIq::Item &item : items) {
		if (item.jid() == m_client->configuration().domain())
			continue;
		m_manager->requestInfo(item.jid());
	}
}
