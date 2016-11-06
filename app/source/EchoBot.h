#ifndef ECHOBOT_H
#define ECHPBOT_H

#include <Swiften/Swiften.h>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"

class RosterController;

class EchoBot
{
public:
    EchoBot(Swift::NetworkFactories* networkFactories);
    ~EchoBot();

private:
    void handlePresenceReceived(Swift::Presence::ref presence);
    void handleConnected();
    void handleMessageReceived(Swift::Message::ref message);

private:
    Swift::Client* client;
    Swift::ClientXMLTracer* tracer;
    Swift::SoftwareVersionResponder* softwareVersionResponder;
    EchoPayloadParserFactory echoPayloadParserFactory;
    EchoPayloadSerializer echoPayloadSerializer;

    RosterController* rosterController_;

};

#endif
