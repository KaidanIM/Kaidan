#ifndef ECHOBOT_H
#define ECHPBOT_H

#include <QObject>
#include <Swiften/Swiften.h>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"

class RosterController;

class Kaidan : public QObject
{
    Q_OBJECT

public:
    Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
    ~Kaidan();

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
