/*
 *  Kaidan - Cross platform XMPP client
 *
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

RosterController::RosterController(QObject *parent) : QObject(parent), client_(NULL), rosterList_()
{
	//rosterList_.append(new RosterItem(QString("lala@lala.de"), QString("lala"), None));
}

void RosterController::requestRosterFromClient(Swift::Client *client)
{
	client_ = client;

	client_->requestRoster();

	Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client->getIQRouter());
	rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
	rosterRequest->send();
}

void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error)
{
	if (error)
	{
		std::cerr << "Error receiving roster. Continuing anyway.";
	}
	else
	{
		std::cout << "handleRosterReceived!!!" << std::endl;
		Swift::XMPPRoster* roster = client_->getRoster();
		std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();

		std::vector<Swift::XMPPRosterItem>::iterator it;
		std::cout << "size: " << rosterItems.size() << std::endl;

		for(it = rosterItems.begin(); it < rosterItems.end(); it++ )
		{
#if 0
			std::cout << "jid: " << (*it).getJID().toString() <<
				", Name: " << (*it).getName() <<
				", Subscription: " << (*it).getSubscription() << std::endl;
#endif

			rosterList_.append(new RosterItem(QString::fromStdString((*it).getJID().toString()),
				QString::fromStdString((*it).getName()),
				(Subscription)(*it).getSubscription()));

			emit rosterListChanged();
		}
	}
}

QQmlListProperty<RosterItem> RosterController::getRosterList()
{
	return QQmlListProperty<RosterItem>(this, rosterList_);
}
