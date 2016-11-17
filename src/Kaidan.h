#ifndef KAIDAN_H
#define KAIDAN_H

#include <QObject>
#include <QStringList>

#include <Swiften/Swiften.h>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"

class RosterController;

class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)

public:
	Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Kaidan();
	Q_INVOKABLE void mainQuit();

	RosterController* getRosterController();

signals:
	void rosterControllerChanged();

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	void handleConnected();
	void handleMessageReceived(Swift::Message::ref message);

	Swift::Client* client;
	Swift::ClientXMLTracer* tracer;
	Swift::SoftwareVersionResponder* softwareVersionResponder;
	EchoPayloadParserFactory echoPayloadParserFactory;
	EchoPayloadSerializer echoPayloadSerializer;

	RosterController* rosterController_;
};

#endif
