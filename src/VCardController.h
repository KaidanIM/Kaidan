/*
 *  Kaidan - Cross platform XMPP client
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

#ifndef VCARDCONTROLLER_H
#define VCARDCONTROLLER_H

// Qt
#include <QObject>
// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/VCard.h>
#include <Swiften/JID/JID.h>
// Kaidan
#include "VCard.h"

class VCardController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString currentJid READ getCurrentJid WRITE setCurrentJid)
	Q_PROPERTY(VCard* currentVCard READ getCurrentVCard NOTIFY currentVCardChanged)
	Q_PROPERTY(VCard* ownVCard READ getOwnVCard WRITE setOwnVCard NOTIFY ownVCardChanged)

public:
	VCardController(QObject *parent = 0);
	~VCardController();

	void setClient(Swift::Client *client_);

	QString getCurrentJid();
	void setCurrentJid(QString);
	VCard* getCurrentVCard();
	VCard* getOwnVCard();
	void setOwnVCard(VCard*);

signals:
	void currentVCardChanged();
	void ownVCardChanged();

private:
	void handleVCardChanged(const Swift::JID &jid_, Swift::VCard::ref vCard_);
	void handleOwnVCardChanged(Swift::VCard::ref vCard_);
	void requestCurrentVCard();
	void requestOwnVCard();

	Swift::Client* client;
	Swift::VCardManager* manager;
	VCard* currentVCard;
	VCard* ownVCard;
	QString currentJid;
};

#endif // VCARDCONTROLLER_H
