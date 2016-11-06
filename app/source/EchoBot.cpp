#include "EchoBot.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "EchoPayload.h"
#include "RosterContoller.h"

EchoBot::EchoBot(NetworkFactories* networkFactories) : rosterController_(NULL)
{
    client = new Swift::Client("schorsch@jabber-germany.de", "J6$er4ey", networkFactories);
    client->setAlwaysTrustCertificates();
    client->onConnected.connect(boost::bind(&EchoBot::handleConnected, this));
    client->onMessageReceived.connect(
                boost::bind(&EchoBot::handleMessageReceived, this, _1));
    client->onPresenceReceived.connect(
                boost::bind(&EchoBot::handlePresenceReceived, this, _1));
    tracer = new Swift::ClientXMLTracer(client);

    softwareVersionResponder = new Swift::SoftwareVersionResponder(client->getIQRouter());
    softwareVersionResponder->setVersion("Kaidan", "0.1");
    softwareVersionResponder->start();
    //...
    client->addPayloadParserFactory(&echoPayloadParserFactory);
    client->addPayloadSerializer(&echoPayloadSerializer);
    //...
    client->connect();
    //...
}

EchoBot::~EchoBot()
{
    client->removePayloadSerializer(&echoPayloadSerializer);
    client->removePayloadParserFactory(&echoPayloadParserFactory);
    //...
    softwareVersionResponder->stop();
    delete softwareVersionResponder;
    delete tracer;
    delete client;

    delete rosterController_;
    //...
}
//...


void EchoBot::handlePresenceReceived(Presence::ref presence)
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

void EchoBot::handleConnected()
{
    client->sendPresence(Presence::create("Send me a message"));

    if (rosterController_ == NULL)
    {
        // Request the roster
        rosterController_ = new RosterController(client);
    }
}

//...
void EchoBot::handleMessageReceived(Message::ref message)
{
    //...
    // Echo back the incoming message
    message->setTo(message->getFrom());
    message->setFrom(JID());
    //...
    if (!message->getPayload<EchoPayload>())
    {
        boost::shared_ptr<EchoPayload> echoPayload = boost::make_shared<EchoPayload>();
        echoPayload->setMessage("This is an echoed message");
        message->addPayload(echoPayload);
        client->sendMessage(message);
    }
}

