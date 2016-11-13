#include "Kaidan.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "EchoPayload.h"
#include "RosterContoller.h"

Kaidan::Kaidan(NetworkFactories* networkFactories, QObject *parent) :
	rosterController_(new RosterController()), QObject(parent)
{
	client = new Swift::Client("jid@...", "pass", networkFactories);
	client->setAlwaysTrustCertificates();
	client->onConnected.connect(boost::bind(&Kaidan::handleConnected, this));
	client->onMessageReceived.connect(
		boost::bind(&Kaidan::handleMessageReceived, this, _1));
	client->onPresenceReceived.connect(
		boost::bind(&Kaidan::handlePresenceReceived, this, _1));
	tracer = new Swift::ClientXMLTracer(client);

	softwareVersionResponder = new Swift::SoftwareVersionResponder(client->getIQRouter());
	softwareVersionResponder->setVersion("Kaidan", "0.1");
	softwareVersionResponder->start();

	client->addPayloadParserFactory(&echoPayloadParserFactory);
	client->addPayloadSerializer(&echoPayloadSerializer);

	client->connect();
}

Kaidan::~Kaidan()
{
	client->removePayloadSerializer(&echoPayloadSerializer);
	client->removePayloadParserFactory(&echoPayloadParserFactory);

	softwareVersionResponder->stop();
	delete softwareVersionResponder;
	delete tracer;
	delete client;

	delete rosterController_;
}
//we don't want to close client without disconnection
void Kaidan::mainQuit(){
	client->disconnect();
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
	client->sendPresence(Presence::create("Send me a message"));

	// Request the roster
	rosterController_->requestRosterFromClient(client);
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
