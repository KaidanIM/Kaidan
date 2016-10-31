#ifndef ECHOBOT_H
#define ECHPBOT_H

#include <Swiften/Swiften.h>
#include <QObject>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"

class EchoBot
{
    //Q_OBJECT

public:
    EchoBot(Swift::NetworkFactories* networkFactories);
    ~EchoBot();

private slots:
    void handlePresenceReceived(Swift::Presence::ref presence);
    void handleConnected();
    void handleRosterReceived(Swift::ErrorPayload::ref error);
    void handleMessageReceived(Swift::Message::ref message);

private:
    Swift::Client* client;
    Swift::ClientXMLTracer* tracer;
    Swift::SoftwareVersionResponder* softwareVersionResponder;
    EchoPayloadParserFactory echoPayloadParserFactory;
    EchoPayloadSerializer echoPayloadSerializer;

};

#endif
