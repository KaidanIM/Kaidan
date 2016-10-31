#include "EchoBot.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include "EchoPayload.h"

using namespace boost;

// FIXME rewrite all binds to qt connects

EchoBot::EchoBot(NetworkFactories* networkFactories)
{
    client = new Client("schorsch@jabber-germany.de", "jwillrein", networkFactories);
    client->setAlwaysTrustCertificates();
    client->onConnected.connect(bind(&EchoBot::handleConnected, this));
    client->onMessageReceived.connect(
                bind(&EchoBot::handleMessageReceived, this, _1));
    client->onPresenceReceived.connect(
                bind(&EchoBot::handlePresenceReceived, this, _1));
    tracer = new ClientXMLTracer(client);

    softwareVersionResponder = new SoftwareVersionResponder(client->getIQRouter());
    softwareVersionResponder->setVersion("EchoBot", "1.0");
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
    //...
}
//...


void EchoBot::handlePresenceReceived(Presence::ref presence)
{
    // Automatically approve subscription requests
    if (presence->getType() == Presence::Subscribe)
    {
        Presence::ref response = Presence::create();
        response->setTo(presence->getFrom());
        response->setType(Presence::Subscribed);
        client->sendPresence(response);
    }
}

void EchoBot::handleConnected()
{
    // Request the roster
    GetRosterRequest::ref rosterRequest =
            GetRosterRequest::create(client->getIQRouter());
    rosterRequest->onResponse.connect(
                bind(&EchoBot::handleRosterReceived, this, _2));
    rosterRequest->send();
}

void EchoBot::handleRosterReceived(ErrorPayload::ref error)
{
    if (error)
    {
        std::cerr << "Error receiving roster. Continuing anyway.";
    }
    // Send initial available presence
    client->sendPresence(Presence::create("Send me a message"));
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

