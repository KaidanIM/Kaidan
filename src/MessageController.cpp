/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
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

#include "MessageController.h"

// Qt
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlQuery>
// Boost
#include <boost/bind.hpp>
#include <boost/smart_ptr/make_shared.hpp>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "EchoPayload.h"
#include "MessageModel.h"

MessageController::MessageController(Swift::Client* client_, QObject *parent) : QObject(parent)
{
	client = client_;
	client->onMessageReceived.connect(boost::bind(&MessageController::handleMessageReceived, this, _1));

	messageModel = new MessageModel();
	emit messageModelChanged();
}

MessageController::~MessageController()
{

}

MessageModel* MessageController::getMessageModel()
{
	return messageModel;
}

void MessageController::setRecipient(QString recipient_)
{
	if (recipient == recipient_)
		return;

	recipient = recipient_;
	emit recipientChanged();

	messageModel->applyRecipientFilter(recipient);
	emit messageModelChanged();
}

QString MessageController::getRecipient()
{
	return recipient;
}


void MessageController::handleMessageReceived(Swift::Message::ref message_)
{
	// check if the message has a body -> is valid
	if (!message_->getBody())
	{
		std::cout << "MessageController: Received message without body." << '\n';
		return;
	}
	else
	{
		std::cout << "MessageController: Message received." << '\n';
	}


	//
	// add the message to the db
	//

	// author is only the 'bare' JID: e.g. 'albert@einstein.ch'
	const QString author = QString(message_->getFrom().toBare().toString().c_str());
	const QString author_resource = QString(message_->getFrom().getResource().c_str());
	const QString recipient = QString("Me");
	const QString recipient_resource = QString("");
	const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
	const QString message = QString(message_->getBody()->c_str());

	messageModel->addMessage(&author, &author_resource, &recipient,
		&recipient_resource, &timestamp, &message);

	emit messageModelChanged();
}

void MessageController::sendMessage(const QString recipient_, const QString message_)
{
	//
	// add the message to the db
	//

	const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
	const QString author = QString("Me");
	const QString author_resource = QString(client->getJID().getResource().c_str());
	const QString recipient_resource = QString("");

	messageModel->addMessage(&author, &author_resource, &recipient_,
		&recipient_resource, &timestamp, &message_);

	emit messageModelChanged();


	//
	// send the message
	//

	boost::shared_ptr<Swift::Message> newMessage(new Swift::Message());
	newMessage->setTo(Swift::JID(recipient_.toStdString()));
	newMessage->setFrom(client->getJID());
	newMessage->setBody(message_.toStdString());

	// send the message
	client->sendMessage(newMessage);

	std::cout << "MessageController: Sent new message from " << newMessage->getFrom() << " to " << newMessage->getTo() << '\n';
}
