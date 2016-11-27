#ifndef KAIDAN_H
#define KAIDAN_H

// Qt
#include <QObject>
#include <QStringList>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"
#include "Persistence.h"


class RosterController;
class Persistence;

class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
	Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
	Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)

public:
	Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Kaidan();

	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
	Q_INVOKABLE void sendMessage(QString const &toJid, QString const &message);
	Q_INVOKABLE void setCurrentChatPartner(QString const &jid);

	bool connectionState() const;

signals:
	void rosterControllerChanged();
	void persistenceChanged();

	void connectionStateConnected();
	void connectionStateDisconnected();

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	void handleConnected();
	void handleDisconnected();
	void handleMessageReceived(Swift::Message::ref message);
	bool connected;

	RosterController* getRosterController();
	Persistence* getPersistence();

	Swift::Client* client_;
	Swift::ClientXMLTracer* tracer_;
	Swift::SoftwareVersionResponder* softwareVersionResponder_;
	EchoPayloadParserFactory echoPayloadParserFactory_;
	EchoPayloadSerializer echoPayloadSerializer_;
	Swift::NetworkFactories *netFactories_;

	RosterController* rosterController_;
	Persistence* persistence_;
};

#endif // KAIDAN_H
