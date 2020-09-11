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

#pragma once

#include <QObject>
#include <QString>
#include <QXmppVCardManager.h>
#include <QXmppPresence.h>

class AvatarFileStorage;
class ClientWorker;
class QXmppClient;

class VCardManager : public QObject
{
	Q_OBJECT

public:
	VCardManager(ClientWorker *clientWorker, QXmppClient *client, AvatarFileStorage *avatars, QObject *parent = nullptr);

	/**
	 * Requests the vCard of a given JID from the JID's server.
	 *
	 * @param jid JID for which the vCard is being requested
	 */
	void requestVCard(const QString &jid);

	/**
	 * Handles an incoming vCard and processes it like saving a containing user avatar etc..
	 *
	 * @param iq IQ stanza containing the vCard
	 */
	void handleVCardReceived(const QXmppVCardIq &iq);

	/**
	 * Requests the user's vCard from the server.
	 */
	void requestClientVCard();

	/**
	 * Handles the receiving of the user's vCard.
	 */
	void handleClientVCardReceived();

	/**
	 * Handles an incoming presence stanza and checks if the user avatar needs to be refreshed.
	 *
	 * @param presence a contact's presence stanza
	 */
	void handlePresenceReceived(const QXmppPresence &presence);

	/**
	 * Executes a pending nickname change if the nickname could not be changed on the
	 * server before because the client was disconnected.
	 *
	 * @return true if the pending nickname change is executed on the second login with
	 * the same credentials or later, otherwise false
	 */
	bool executePendingNicknameChange();

public slots:
	/**
	 * Changes the user's nickname.
	 *
	 * @param nickname name that is shown to contacts after the update
	 */
	void changeNickname(const QString &nickname);

signals:
	/**
	 * Emitted when any vCard is received.
	 *
	 * @param vCard received vCard
	 */
	void vCardReceived(const QXmppVCardIq &vCard);

private:
	/**
	 * Changes the nickname which was cached to be set after receiving the current vCard.
	 */
	void changeNicknameAfterReceivingCurrentVCard();

	ClientWorker *m_clientWorker;
	QXmppClient *m_client;
	QXmppVCardManager *m_manager;
	AvatarFileStorage *m_avatarStorage;
	QString m_nicknameToBeSetAfterReceivingCurrentVCard;
};
