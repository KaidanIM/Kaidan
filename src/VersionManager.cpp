/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

#include "VersionManager.h"

#include <QStringBuilder>

#include <QXmppClient.h>
#include <QXmppVersionManager.h>
#include <QXmppRosterManager.h>
#include <QXmppPresence.h>

#include "Kaidan.h"

VersionManager::VersionManager(QXmppClient *client, QObject *parent)
	: QObject(parent),
	  m_manager(client->findExtension<QXmppVersionManager>()),
	  m_client(client)
{
	Q_ASSERT(m_manager);

	// publish kaidan version
	m_manager->setClientName(APPLICATION_DISPLAY_NAME);
	m_manager->setClientVersion(VERSION_STRING);
	m_manager->setClientOs(QSysInfo::prettyProductName());

	connect(m_manager, &QXmppVersionManager::versionReceived,
		this, &VersionManager::clientVersionReceived);
}

void VersionManager::fetchVersions(const QString &bareJid, const QString &resource)
{
	const auto fetchVersion = [this, &bareJid](const QString &res) {
		m_manager->requestVersion(bareJid % u'/' % res);
	};

	if (resource.isEmpty()) {
		const auto resources = m_client->findExtension<QXmppRosterManager>()->getResources(bareJid);
		std::for_each(resources.cbegin(), resources.cend(), fetchVersion);
	} else {
		fetchVersion(resource);
	}
}
