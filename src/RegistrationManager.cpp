/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

#include "RegistrationManager.h"
#include "Globals.h"
#include "Kaidan.h"

#include <QXmppRegisterIq.h>
#include <QXmppClient.h>
#include <QXmppUtils.h>
#include <QXmppDiscoveryManager.h>

#include <QSettings>
#include <QDomElement>

RegistrationManager::RegistrationManager(Kaidan *kaidan, QSettings *settings)
        : kaidan(kaidan), settings(settings)
{
	connect(kaidan, &Kaidan::changePassword, this, &RegistrationManager::changePassword);
	connect(this, &RegistrationManager::passwordChanged, kaidan, &Kaidan::setPassword);
	connect(this, &RegistrationManager::passwordChanged,
	        kaidan, &Kaidan::passwordChangeSucceeded);
	connect(this, &RegistrationManager::passwordChangeFailed,
	        kaidan, &Kaidan::passwordChangeFailed);
}

QStringList RegistrationManager::discoveryFeatures() const
{
	return QStringList() << NS_REGISTER;
}

void RegistrationManager::changePassword(const QString &newPassword)
{
	m_newPasswordIqId = QXmppUtils::generateStanzaHash();
	m_newPassword = newPassword;

	QXmppRegisterIq iq;
	iq.setType(QXmppIq::Set);
	iq.setTo(client()->configuration().domain());
	iq.setFrom(client()->configuration().jid());
	iq.setUsername(QXmppUtils::jidToUser(client()->configuration().jid()));
	iq.setPassword(newPassword);
	iq.setId(m_newPasswordIqId);

	client()->sendPacket(iq);
}

bool RegistrationManager::handleStanza(const QDomElement &stanza)
{
	// result of change password:
	if (!m_newPassword.isEmpty() && stanza.attribute("id") == m_newPasswordIqId) {
		QXmppRegisterIq iq;
		iq.parse(stanza);

		if (iq.type() == QXmppIq::Result) {
			// Success
			client()->configuration().setPassword(m_newPassword);
			settings->setValue(KAIDAN_SETTINGS_AUTH_PASSWD,
			                   QString::fromUtf8(m_newPassword.toUtf8().toBase64()));
			emit passwordChanged(m_newPassword);
			emit kaidan->passiveNotificationRequested(
			    tr("Password changed successfully.")
			);

		} else if (iq.type() == QXmppIq::Error) {
			// Error
			emit passwordChangeFailed();
			emit kaidan->passiveNotificationRequested(
			    tr("Failed to change password: %1").arg(iq.error().text())
			);
			qWarning() << QString("Failed to change password: %1").arg(iq.error().text());
		}
		m_newPassword = "";
		m_newPasswordIqId = "";
		return true;
	}
	return false;
}

void RegistrationManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
	// check features of own server
	if (iq.from().isEmpty() || iq.from() == client()->configuration().domain()) {
		if (iq.features().contains(NS_REGISTER))
			setRegistrationSupported(true);
	}
}

bool RegistrationManager::registrationSupported() const
{
	return m_registrationSupported;
}

void RegistrationManager::setRegistrationSupported(bool registrationSupported)
{
	if (m_registrationSupported == registrationSupported) {
		m_registrationSupported = registrationSupported;
		emit registrationSupportedChanged();
	}
}

void RegistrationManager::setClient(QXmppClient *client)
{
	QXmppClientExtension::setClient(client);
	// get service discovery manager
	auto *disco = client->findExtension<QXmppDiscoveryManager>();
	if (disco) {
		connect(disco, &QXmppDiscoveryManager::infoReceived,
		        this, &RegistrationManager::handleDiscoInfo);

		connect(client, &QXmppClient::disconnected, this, [=] () {
			setRegistrationSupported(false);
		});
	}
}
