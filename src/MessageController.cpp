/*
 *  Kaidan - A user-friendly XMPP client for every device!
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
// Boost
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <boost/smart_ptr/make_shared.hpp>
// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/Message.h>
#include <Swiften/Elements/DeliveryReceipt.h>
#include <Swiften/Elements/DeliveryReceiptRequest.h>
#include <Swiften/Base/IDGenerator.h>
// Kaidan
#include "MessageModel.h"
#include "Notifications.h"

MessageController::MessageController(QSqlDatabase *database, QObject *parent) : QObject(parent)
{
	messageModel = new MessageModel(database);
	emit messageModelChanged();
}

MessageController::~MessageController()
{
	delete messageModel;
}

void MessageController::setClient(Swift::Client* client_)
{
	client = client_;
	client->onMessageReceived.connect(boost::bind(&MessageController::handleMessageReceived, this, _1));
}

MessageModel* MessageController::getMessageModel()
{
	return messageModel;
}

void MessageController::setChatPartner(QString *recipient, QString* ownJid)
{
	// we have to use ownJid here, because this should also be usable when
	// we're offline or we haven't connected already.
	messageModel->applyRecipientFilter(recipient, ownJid);
}

void MessageController::handleMessageReceived(Swift::Message::ref message_)
{
	boost::optional<std::string> bodyOpt = message_->getBody();

	if (bodyOpt) {
		//
		// add the message to the db
		//

		// author is only the 'bare' JID: e.g. 'albert@einstein.ch'
		const QString author = QString::fromStdString(message_->getFrom().toBare().toString());
		const QString author_resource = QString::fromStdString(message_->getFrom().getResource());
		const QString recipient = QString::fromStdString(message_->getTo().toBare().toString());
		const QString recipient_resource = QString::fromStdString(client->getJID().getResource());
		QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate); // fallback timestamp
		const QString message = QString::fromStdString(*bodyOpt);
		const QString msgId = QString::fromStdString(message_->getID());

		// get the timestamp from the message, if exists
		boost::optional<boost::posix_time::ptime> timestampOpt = message_->getTimestamp();
		if (timestampOpt) {
			timestamp = QString::fromStdString(
			                    boost::posix_time::to_iso_extended_string(*timestampOpt)
			            );
		}

		messageModel->addMessage(&author, &author_resource, &recipient,
			&recipient_resource, &timestamp, &message, &msgId, false);

		// send a new notification | TODO: Resolve nickname from JID
		Notifications::sendMessageNotification(
		        message_->getFrom().toBare().toString(),
		        *bodyOpt
		);
	}

	// XEP-0184: Message Delivery Receipts
	// send a reply that the message has arrived
	if (message_->getPayload<Swift::DeliveryReceiptRequest>()) {
		// create a new reply payload
		boost::shared_ptr<Swift::DeliveryReceipt> receiptPayload =
		        boost::make_shared<Swift::DeliveryReceipt>();
		receiptPayload->setReceivedID(message_->getID());

		// create a new message
		Swift::Message::ref receiptReply = boost::make_shared<Swift::Message>();
		receiptReply->setFrom(message_->getTo());
		receiptReply->setTo(message_->getFrom());

		// add the reply payload to the message
		receiptReply->addPayload(receiptPayload);

		// send the message
		client->sendMessage(receiptReply);
	}

	// XEP-0184: Message Delivery Receipts
	// get a reply of a delivered receipt request
	Swift::DeliveryReceipt::ref receipt = message_->getPayload<Swift::DeliveryReceipt>();
	if (receipt) {
		std::string receivedId = receipt->getReceivedID();
		if (receivedId.length() > 0) {
			messageModel->setMessageAsDelivered(QString::fromStdString(receivedId));
		}
	}
}

void MessageController::sendMessage(QString *fromJid, QString *recipient_, QString *message_)
{
	// generate a new message id
	Swift::IDGenerator idGenerator;
	std::string msgId = idGenerator.generateID();

	//
	// add the message to the db
	//

	const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
	const QString author_resource = QString::fromStdString(client->getJID().getResource());
	const QString recipient_resource = QString("");
	const QString qmsgId = QString::fromStdString(msgId);

	messageModel->addMessage(fromJid, &author_resource, recipient_,
	                         &recipient_resource, &timestamp, message_, &qmsgId, true);

	//
	// send the message
	//

	boost::shared_ptr<Swift::Message> newMessage(new Swift::Message());
	newMessage->setTo(Swift::JID(recipient_->toStdString()));
	newMessage->setFrom(client->getJID());
	newMessage->setBody(message_->toStdString());
	newMessage->setID(msgId);

	// XEP-0184: Message Delivery Receipts
	// add a delivery receipt request
	newMessage->addPayload(boost::make_shared<Swift::DeliveryReceiptRequest>());

	// send the message
	client->sendMessage(newMessage);
}
