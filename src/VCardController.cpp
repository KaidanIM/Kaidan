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

// VCardController
#include "VCardController.h"
// C++
#include <string>
// Qt 5
#include <QDebug>
// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/VCards/VCardManager.h>
// Boost
#include <boost/bind.hpp>
// Kaidan
#include "VCard.h"

VCardController::VCardController(QObject *parent) : QObject(parent)
{
	currentVCard = new VCard();
	ownVCard = new VCard();
}

VCardController::~VCardController()
{
	delete currentVCard;
	delete ownVCard;
}

void VCardController::setClient(Swift::Client* client_)
{
	client = client_;
	client->onConnected.connect(boost::bind(&VCardController::requestOwnVCard, this));
	client->onConnected.connect(boost::bind(&VCardController::requestCurrentVCard, this));

	manager = client->getVCardManager();
	manager->onVCardChanged.connect(boost::bind(&VCardController::handleVCardChanged, this, _1, _2));
	manager->onOwnVCardChanged.connect(boost::bind(&VCardController::handleOwnVCardChanged, this, _1));
}

void VCardController::setCurrentJid(QString jid_)
{
	// set the JID
	currentJid = jid_;

	// get the new VCard, if client is connected
	if (client->isAvailable()) {
		currentVCard->fromSwiftVCard(manager->getVCardAndRequestWhenNeeded(
		                                     Swift::JID(currentJid.toStdString())
		                             ));
		emit currentVCardChanged();
	}
}

QString VCardController::getCurrentJid()
{
	return currentJid;
}

VCard* VCardController::getCurrentVCard()
{
	return currentVCard;
}

VCard* VCardController::getOwnVCard()
{
	return ownVCard;
}

void VCardController::setOwnVCard(VCard* vCard_)
{
	// publish new vCard
	Swift::SetVCardRequest::ref vCardChangeRequest = manager->createSetVCardRequest(
	                        vCard_->getSwiftVCard());
	vCardChangeRequest->send();
}

void VCardController::requestOwnVCard()
{
	manager->requestOwnVCard();
}

void VCardController::requestCurrentVCard()
{
	// if a new jid was already set, but the client wasn't connected
	if (currentJid != "") {
		currentVCard->fromSwiftVCard(manager->getVCardAndRequestWhenNeeded(
		                                     Swift::JID(currentJid.toStdString())
		                             ));
		emit currentVCardChanged();
	}
}

void VCardController::handleVCardChanged(const Swift::JID &jid_, Swift::VCard::ref vCard_)
{
	std::cout << "VCardController::handleVCardChanged: Received new VCard from: " << jid_.toString() << '\n';
	if (jid_.toString() == currentJid.toStdString()) {
		// update the current VCard
		currentVCard->fromSwiftVCard(vCard_);
		emit currentVCardChanged();
	}
}

void VCardController::handleOwnVCardChanged(Swift::VCard::ref vCard_)
{
	std::cout << "VCardController::handleOwnVCardChanged: Received own VCard" << '\n';
	ownVCard->fromSwiftVCard(vCard_);
	emit ownVCardChanged();
}
