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
	Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)

public:
	Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Kaidan();
	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
	bool connectionState() const;

	RosterController* getRosterController();

signals:
	void rosterControllerChanged();
	void connectionStateConnected();
	void connectionStateDisconnected();

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	void handleConnected();
	void handleDisconnected();
	void handleMessageReceived(Swift::Message::ref message);
	bool connected;

	Swift::Client* client;
	Swift::ClientXMLTracer* tracer;
	Swift::SoftwareVersionResponder* softwareVersionResponder;
	EchoPayloadParserFactory echoPayloadParserFactory;
	EchoPayloadSerializer echoPayloadSerializer;
	Swift::NetworkFactories *netFactories;

	RosterController* rosterController_;
};

#endif
