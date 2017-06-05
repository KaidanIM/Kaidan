/*
 *  Kaidan - A user-friendly XMPP client for every device!
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
// Std
#include <iostream>
#include <string.h>
// Qt 5
#include <QQmlContext>
#include <QDateTime>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
// Kaidan
#include "RosterModel.h"
// Swiften
#include <Swiften/Base/IDGenerator.h>
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/ErrorPayload.h>
#include <Swiften/JID/JID.h>
#include <Swiften/Roster/GetRosterRequest.h>
#include <Swiften/Roster/XMPPRoster.h>
#include <Swiften/Queries/IQRouter.h>
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
	client->onConnected.connect(boost::bind(&RosterController::requestRosterFromClient, this));
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
		// create a vector containing all roster items
		std::vector<Swift::XMPPRosterItem> rosterItems = xmppRoster->getItems();
		// create an iterator for it
		std::vector<Swift::XMPPRosterItem>::iterator it;


		//
		// Find out which/if JIDs have been removed
		//

		// list of JIDs from server
		QStringList newJids;
		for (it = rosterItems.begin(); it < rosterItems.end(); it++)
			newJids << QString::fromStdString((*it).getJID().toBare().toString());

		// list of the JIDs from the DB
		QStringList currentJids = rosterModel->getJidList();
		// a new list with all JIDs to delete
		QStringList jidsToDelete;

		// add all JIDs to the delete list that are in the original list
		// but not in the new from the server
		for (int i = 0; i < currentJids.length(); i++)
		{
			QString jidAtI = currentJids.at(i);
			if (!newJids.contains(jidAtI))
			{
				jidsToDelete << jidAtI;
			}
		}

		// remove the JIDs from the DB
		rosterModel->removeListOfJids(&jidsToDelete);

		//
		// Update the roster
		//

		for (it = rosterItems.begin(); it < rosterItems.end(); it++)
		{
			QString jid = QString::fromStdString((*it).getJID().toBare().toString());
			QString name = QString::fromStdString((*it).getName());

			if (currentJids.contains(jid))
			{
				rosterModel->updateContactName(jid, name);
			}
			else
			{
				rosterModel->insertContact(jid, name);
			}
		}

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

void RosterController::updateLastExchangedOfJid(const QString jid_)
{
	rosterModel->setLastExchangedOfJid(jid_, QDateTime::currentDateTime().toString(Qt::ISODate));

	// send signal for updating the GUI
	emit rosterModelChanged();
}

void RosterController::newUnreadMessageForJid(const QString jid_)
{
	// get the current unread message count
	int msgCount = rosterModel->getUnreadMessageCountOfJid(&jid_);
	// increase it by one
	msgCount++;
	// set the new increased count
	rosterModel->setUnreadMessageCountOfJid(&jid_, msgCount);
}

void RosterController::resetUnreadMessagesForJid(const QString jid_)
{
	rosterModel->setUnreadMessageCountOfJid(&jid_, 0);
	emit rosterModelChanged();
}
