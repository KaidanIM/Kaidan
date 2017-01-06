/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
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

#include "Kaidan.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "EchoPayload.h"
#include "RosterController.h"

Kaidan::Kaidan(NetworkFactories* networkFactories, QObject *parent) :
	rosterController_(new RosterController()), QObject(parent)
{
	netFactories = networkFactories;
	connected = false;
}

Kaidan::~Kaidan()
{
	if (connected)
	{
		client->removePayloadSerializer(&echoPayloadSerializer);
		client->removePayloadParserFactory(&echoPayloadParserFactory);
		softwareVersionResponder->stop();
		delete tracer;
		delete softwareVersionResponder;
		delete client;
	}

	delete rosterController_;
}

void Kaidan::mainConnect(const QString &jid, const QString &pass){
	client = new Swift::Client(jid.toStdString(), pass.toStdString(), netFactories);
	client->setAlwaysTrustCertificates();
	client->onConnected.connect(boost::bind(&Kaidan::handleConnected, this));
	client->onDisconnected.connect(boost::bind(&Kaidan::handleDisconnected, this));
	client->onMessageReceived.connect(
		boost::bind(&Kaidan::handleMessageReceived, this, _1));
	client->onPresenceReceived.connect(
		boost::bind(&Kaidan::handlePresenceReceived, this, _1));
	tracer = new Swift::ClientXMLTracer(client);

	softwareVersionResponder = new Swift::SoftwareVersionResponder(client->getIQRouter());
	softwareVersionResponder->setVersion(APPLICATION_DISPLAY_NAME, VERSION_STRING);
	softwareVersionResponder->start();

	client->addPayloadParserFactory(&echoPayloadParserFactory);
	client->addPayloadSerializer(&echoPayloadSerializer);

	client->connect();
}

//we don't want to close client without disconnection
void Kaidan::mainDisconnect()
{
	if (connectionState())
	{
		client->disconnect();
	}
}

void Kaidan::handlePresenceReceived(Presence::ref presence)
{
	// Automatically approve subscription requests
	if (presence->getType() == Swift::Presence::Subscribe)
	{
		Swift::Presence::ref response = Swift::Presence::create();
		response->setTo(presence->getFrom());
		response->setType(Swift::Presence::Subscribed);
		client->sendPresence(response);
	}
}

void Kaidan::handleConnected()
{
	connected = true;
	emit connectionStateConnected();
	client->sendPresence(Presence::create("Send me a message"));

	// Request the roster
	rosterController_->requestRosterFromClient(client);
}

void Kaidan::handleDisconnected()
{
	connected = false;
	emit connectionStateDisconnected();
}

void Kaidan::handleMessageReceived(Message::ref message)
{
	// Echo back the incoming message
	message->setTo(message->getFrom());
	message->setFrom(JID());

	if (!message->getPayload<EchoPayload>())
	{
		boost::shared_ptr<EchoPayload> echoPayload = boost::make_shared<EchoPayload>();
		echoPayload->setMessage("This is an echoed message");
		message->addPayload(echoPayload);
		client->sendMessage(message);
	}
}

RosterController* Kaidan::getRosterController()
{
	return rosterController_;
}

bool Kaidan::connectionState() const
{
	return connected;
}
