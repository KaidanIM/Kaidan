/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
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

// RosterController
#include "RosterController.h"
// C++
#include <string.h>
// Qt 5
#include <QQmlContext>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
// Kaidan
#include "RosterModel.h"
// Swiften
#include <Swiften/Swiften.h>
// Boost
#include <boost/bind.hpp>

RosterController::RosterController(QObject *parent) : QObject(parent)
{
	rosterModel = new RosterModel();
}

RosterController::~RosterController()
{
	delete rosterModel;
}

void RosterController::setClient(Swift::Client *client_)
{
	client = client_;
	iqRouter = client->getIQRouter();
	xmppRoster = client->getRoster();
	xmppRoster->onInitialRosterPopulated.connect(boost::bind(&RosterController::handleInitialRosterPopulated, this));
}

RosterModel* RosterController::getRosterModel()
{
	return rosterModel;
}

void RosterController::requestRosterFromClient()
{
	client->requestRoster();

	Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client->getIQRouter());
	rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
	rosterRequest->send();
}

void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error_)
{
	if (error_)
	{
		std::cerr << "RosterController: Error receiving roster. Continuing anyway.\n";
	}
	else
	{
		// remove all rows / contacts from the model (we've got new ones)
		rosterModel->clearData();

		// create a vector containing all roster items
		std::vector<Swift::XMPPRosterItem> rosterItems = xmppRoster->getItems();
		// create an iterator for it
		std::vector<Swift::XMPPRosterItem>::iterator it;

		// add all contacts from the received roster
		for(it = rosterItems.begin(); it < rosterItems.end(); it++)
		{
			rosterModel->insertContact(
				QString::fromStdString((*it).getJID().toBare().toString()),
				QString::fromStdString((*it).getName())
			);
		}

		// submit all new changes
		rosterModel->submitAll();
		// send signal for updating the GUI
		emit rosterModelChanged();
	}
}

void RosterController::handleInitialRosterPopulated()
{
	// all contacts from the roster were added to the db (handleRosterReceived was already called)
	// for all later changes we connect these bindings:
	xmppRoster->onJIDAdded.connect(boost::bind(&RosterController::handleJidAdded, this, _1));
	xmppRoster->onJIDRemoved.connect(boost::bind(&RosterController::handleJidRemoved, this, _1));
	xmppRoster->onJIDUpdated.connect(boost::bind(&RosterController::handleJidUpdated, this, _1, _2, _3));
	xmppRoster->onRosterCleared.connect(boost::bind(&RosterController::handleRosterCleared, this));
}

void RosterController::handleJidAdded(const Swift::JID &jid_)
{
	rosterModel->insertContact(
		QString::fromStdString(jid_.toBare().toString()),
		QString::fromStdString(xmppRoster->getNameForJID(jid_))
	);

	rosterModel->submitAll();

	emit rosterModelChanged();
}

void RosterController::handleJidRemoved(const Swift::JID &jid_)
{
	rosterModel->removeContactByJid(
		QString::fromStdString(jid_.toBare().toString())
	);

	emit rosterModelChanged();
}

void RosterController::handleJidUpdated(const Swift::JID &jid_, const std::string &name_,
	const std::vector<std::string>&)
{
	rosterModel->updateContactName(
		QString::fromStdString(jid_.toBare().toString()),
		QString::fromStdString(name_)
	);

	emit rosterModelChanged();
}

void RosterController::handleRosterCleared()
{
	// remove all contacts
	rosterModel->clearData();

	emit rosterModelChanged();
}

void RosterController::addContact(const QString jid_, const QString name_)
{
	// the contact will be added to the model via. handleJidAdded

	// generate a new ID for the subscription request
	Swift::IDGenerator idGenerator;
	std::string iqId = idGenerator.generateID();

	// create a new roster item payload
	Swift::RosterItemPayload addItemPayload;
	addItemPayload.setJID(jid_.toStdString());
	addItemPayload.setName(name_.toStdString());
	addItemPayload.setSubscription(Swift::RosterItemPayload::None);

	// add the new roster item payload to a new roster payload
	boost::shared_ptr<Swift::RosterPayload> rosterPayload(new Swift::RosterPayload);
	rosterPayload->addItem(addItemPayload);

	// sent the request
	iqRouter->sendIQ(
		Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), iqId, rosterPayload)
	);
}

void RosterController::removeContact(const QString jid_)
{
	// the contact will be removed from the model via. handleJidRemoved

	// generate new id for the request
	Swift::IDGenerator idGenerator;
	std::string iqId = idGenerator.generateID();

	// create new roster payload, add roster item removal
	boost::shared_ptr<Swift::RosterPayload> rosterPayload(new Swift::RosterPayload);
	rosterPayload->addItem(Swift::RosterItemPayload(
		Swift::JID(jid_.toStdString()), "",
		Swift::RosterItemPayload::Remove
	));

	// send the remove request
	iqRouter->sendIQ(
		Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), iqId, rosterPayload)
	);
}
