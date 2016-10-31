#ifndef ECHOBOT_H
#define ECHPBOT_H

#include <Swiften/Swiften.h>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"

using namespace Swift;
using namespace boost;



class EchoBot
{
public:
    EchoBot(NetworkFactories* networkFactories);
    ~EchoBot();

private:
    void handlePresenceReceived(Presence::ref presence);
    void handleConnected();
    void handleRosterReceived(ErrorPayload::ref error);
    void handleMessageReceived(Message::ref message);

    Client* client;
    ClientXMLTracer* tracer;
    SoftwareVersionResponder* softwareVersionResponder;
    EchoPayloadParserFactory echoPayloadParserFactory;
    EchoPayloadSerializer echoPayloadSerializer;

};

#endif
