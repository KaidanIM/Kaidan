/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Kaidan.h"

#include <iostream>
// Boost
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>
// Qt
#include <QDateTime>
#include <QDebug>
// Kaidan
#include "EchoPayload.h"
#include "RosterController.h"
#include "Persistence.h"
#include "MessageController.h"


Kaidan::Kaidan(NetworkFactories* networkFactories, QObject *parent) :
	QObject(parent), rosterController_(new RosterController())
{
	netFactories_ = networkFactories;
	connected = false;
}

Kaidan::~Kaidan()
{
	if (connected)
	{
		client_->removePayloadSerializer(&echoPayloadSerializer_);
		client_->removePayloadParserFactory(&echoPayloadParserFactory_);
		softwareVersionResponder_->stop();
		//delete tracer_;
		delete softwareVersionResponder_;
		delete client_;
	}

	delete rosterController_;
}

void Kaidan::mainConnect(const QString &jid, const QString &pass)
{
	// create a new swiften client
	client_ = new Swift::Client(jid.toStdString(), pass.toStdString(), netFactories_);

	// trust all certificates
	client_->setAlwaysTrustCertificates();

	// set event handling
	client_->onConnected.connect(boost::bind(&Kaidan::handleConnected, this));
	client_->onDisconnected.connect(boost::bind(&Kaidan::handleDisconnected, this));
	client_->onMessageReceived.connect(boost::bind(&Kaidan::handleMessageReceived, this, _1));
	client_->onPresenceReceived.connect(boost::bind(&Kaidan::handlePresenceReceived, this, _1));

	// create XML tracer
	//tracer_ = new Swift::ClientXMLTracer(client_);

	// share project version
	softwareVersionResponder_ = new Swift::SoftwareVersionResponder(client_->getIQRouter());
	softwareVersionResponder_->setVersion("Kaidan", "0.0.1");	// TODO: use vars from build system
	softwareVersionResponder_->start();

	client_->addPayloadParserFactory(&echoPayloadParserFactory_);
	client_->addPayloadSerializer(&echoPayloadSerializer_);

	// start connecting
	client_->connect();
}

void Kaidan::setCurrentChatPartner(QString const &jid)
{
	qDebug() << "setCurrentChatPartner(" << jid << ")";
	persistence_->setCurrentChatPartner(jid);
}

void Kaidan::sendMessage(QString const &toJid, QString const &message)
{
	// new empty message
	Swift::Message::ref msg(new Swift::Message);
	// create receiver jid
	Swift::JID receiverJid(toJid.toStdString());

	// generate a new id
	Swift::IDGenerator idGenerator;
	std::string msgId = idGenerator.generateID();

	// set metadata
	msg->setFrom(JID(client_->getJID()));
	msg->setTo(receiverJid);
	msg->setID(msgId);
	msg->setBody(message.toStdString());
	msg->addPayload(boost::make_shared<DeliveryReceiptRequest>());

	// send the message
	client_->sendMessage(msg);

	// add the message to the db
	persistence_->addMessage(QString::fromStdString(msgId), QString::fromStdString(receiverJid.toBare().toString()), message, 0);
}

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
	// emit new connection state to qml
	emit connectionStateConnected();
	client_->sendPresence(Presence::create("Send me a message"));

	// Request the roster
	rosterController_->requestRosterFromClient(client_);
}

void Kaidan::handleDisconnected()
{
	connected = false;
	// emit new connection state to qml
	emit connectionStateDisconnected();
}

void Kaidan::handleMessageReceived(Message::ref message)
{
	//std::cout << "handleMessageReceived" << std::endl;

	std::string fromJid = message->getFrom().toBare().toString();
	boost::optional<std::string> fromBody = message->getBody();

	// TODO: add message to persistence if body or media received
	if (fromBody)
	{
		std::string body = *fromBody;
		persistence_->addMessage(QString::fromStdString(message->getID()), QString::fromStdString(fromJid), QString::fromStdString(body), 1 );
	}

	// XEP 0184
	if (message->getPayload<DeliveryReceiptRequest>())
	{
		// send message receipt
		Message::ref receiptReply = boost::make_shared<Message>();
		receiptReply->setFrom(message->getTo());
		receiptReply->setTo(message->getFrom());

		boost::shared_ptr<DeliveryReceipt> receipt = boost::make_shared<DeliveryReceipt>();
		receipt->setReceivedID(message->getID());
		receiptReply->addPayload(receipt);
		client_->sendMessage(receiptReply);
	}

	// mark sent msg as received
	DeliveryReceipt::ref rcpt = message->getPayload<DeliveryReceipt>();
	if (rcpt)
	{
		std::string recevideId = rcpt->getReceivedID();
		if (recevideId.length() > 0)
		{
	    		persistence_->markMessageAsReceivedById(QString::fromStdString(recevideId));
		}
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
