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

#include "RosterController.h"

#include <QQmlContext>
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>

#include "RosterModel.h"

RosterController::RosterController(QObject *parent) : QObject(parent)
{
	rosterModel = new RosterModel();
}

RosterController::~RosterController()
{

}

RosterModel* RosterController::getRosterModel()
{
	return rosterModel;
}

void RosterController::requestRosterFromClient(Swift::Client *client_)
{
	client = client_;

	client->requestRoster();

	Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client->getIQRouter());
	rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
	rosterRequest->send();
	std::cout << "RosterController: Sent roster request\n";
}

void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error_)
{
	if (error_)
	{
		std::cerr << "RosterController: Error receiving roster. Continuing anyway.\n";
	}
	else
	{
		// we know we've received the roster, so we get it from the client
		Swift::XMPPRoster* roster = client->getRoster();

		// create a vector containing all roster items
		std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();
		// create a fitting iterator
		std::vector<Swift::XMPPRosterItem>::iterator it;

		// output the size of the vector with the roster items
		std::cout << "RosterController: Received XMPP Roster with "
			<< rosterItems.size() << " contacts." << std::endl;


		// remove all rows / contacts (we've got new ones)
		for (int i = 0; i < rosterModel->rowCount(); ++i)
		{
			rosterModel->removeRow(i);
		}


		// add all contacts from the received roster
		for(it = rosterItems.begin(); it < rosterItems.end(); it++)
		{
			// create a new record
			QSqlRecord newRecord = rosterModel->record();

			// set the data from the received Roster
			newRecord.setValue("jid", QString::fromStdString((*it).getJID().toString()));
			newRecord.setValue("name", QString::fromStdString((*it).getName()));

			// inster the record into the DB (or print error)
			if (!rosterModel->insertRecord(rosterModel->rowCount(), newRecord)) {
				qWarning() << "Failed to save RosterContact into DB:"
					<< rosterModel->lastError().text();
				return;
			}
		}

		// submit all new changes
		rosterModel->submitAll();

		// send signal for updating the GUI
		emit rosterModelChanged();

		std::cout << "RosterController: Finished updating roster contacts." << '\n';
	}
}
