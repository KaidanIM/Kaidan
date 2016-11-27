#include "Kaidan.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <QDateTime>
#include <QDebug>

#include "EchoPayload.h"
#include "RosterContoller.h"
#include "Persistence.h"
#include "MessageController.h"

Kaidan::Kaidan(NetworkFactories* networkFactories, QObject *parent) :
	rosterController_(new RosterController()), QObject(parent)
{
	netFactories = networkFactories;
	connected = false;
}

Kaidan::~Kaidan()
{
	if (connected)
	{
        client_->removePayloadSerializer(&echoPayloadSerializer);
        client_->removePayloadParserFactory(&echoPayloadParserFactory);
		softwareVersionResponder->stop();
		delete tracer;
		delete softwareVersionResponder;
        delete client_;
	}

	delete rosterController_;
}

void Kaidan::mainConnect(const QString &jid, const QString &pass){
    client_ = new Swift::Client(jid.toStdString(), pass.toStdString(), netFactories);
    client_->setAlwaysTrustCertificates();
    client_->onConnected.connect(boost::bind(&Kaidan::handleConnected, this));
    client_->onDisconnected.connect(boost::bind(&Kaidan::handleDisconnected, this));
    client_->onMessageReceived.connect(
		boost::bind(&Kaidan::handleMessageReceived, this, _1));
    client_->onPresenceReceived.connect(
		boost::bind(&Kaidan::handlePresenceReceived, this, _1));
    tracer = new Swift::ClientXMLTracer(client_);

    softwareVersionResponder = new Swift::SoftwareVersionResponder(client_->getIQRouter());
	softwareVersionResponder->setVersion("Kaidan", "0.1");
	softwareVersionResponder->start();

    client_->addPayloadParserFactory(&echoPayloadParserFactory);
    client_->addPayloadSerializer(&echoPayloadSerializer);

    client_->connect();
}

void Kaidan::setCurrentChatPartner(QString const &jid)
{
    persistence_->setCurrentChatPartner(jid);
}

void Kaidan::sendMessage(QString const &toJid, QString const &message)
{
    Swift::Message::ref msg(new Swift::Message);

    Swift::JID receiverJid(toJid.toStdString());

    msg->setFrom(JID());
    msg->setTo(receiverJid);
    msg->setBody(message.toStdString());
    client_->sendMessage(msg);

    persistence_->addMessage(QString::fromStdString(receiverJid.toBare().toString()), message, 0);
}
//we don't want to close client without disconnection
void Kaidan::mainDisconnect()
{
	if (connectionState())
	{
        client_->disconnect();
	}
}

void Kaidan::handlePresenceReceived(Presence::ref presence)
{
	// Automatically approve subscription requests
	if (presence->getType() == Swift::Presence::Subscribe)
	{
		Swift::Presence::ref response = Swift::Presence::create();
		response->setTo(presence->getFrom());
		response->setType(Swift::Presence::Subscribed);
        client_->sendPresence(response);
	}
}

void Kaidan::handleConnected()
{
	connected = true;
	emit connectionStateConnected();
    client_->sendPresence(Presence::create("Send me a message"));

	// Request the roster
    rosterController_->requestRosterFromClient(client_);
}

void Kaidan::handleDisconnected()
{
	connected = false;
	emit connectionStateDisconnected();
}

void Kaidan::handleMessageReceived(Message::ref message)
{
    std::cout << "handleMessageReceived" << std::endl;

    std::string fromJid = message->getFrom().toBare().toString();
    boost::optional<std::string> fromBody = message->getBody();

#if 0
    QDateTime timeFromMessage;
    boost::optional<boost::posix_time::ptime> tsFromMessage = message->getTimestamp();
    if(tsFromMessage)
    {
        boost::posix_time::ptime ts = *tsFromMessage;
        //qDebug() << "ts: " << ts.time_of_day().
        std::string isoString = boost::posix_time::to_iso_string(ts);
        qDebug() << "isoString: " << isoString.c_str();
        timeFromMessage = QDateTime::fromString(isoString.c_str(), "yyyyMMddTHHmmss");
        qDebug() << "qstring: " << timeFromMessage.toString();
    }
#endif

    // fixme. add empty message if no body in here.
    if (fromBody)
    {
        std::string body = *fromBody;
        persistence_->addMessage(QString::fromStdString(fromJid), QString::fromStdString(body), 1 );
    }
}

RosterController* Kaidan::getRosterController()
{
	return rosterController_;
}

Persistence* Kaidan::getPersistence()
{
    return persistence_;
}

bool Kaidan::connectionState() const
{
	return connected;
}
