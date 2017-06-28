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

#include "MessageHandler.h"
// Std
#include <exception>
#include <iostream>
// Qt
#include <QDateTime>
#include <QDebug>
#include <QString>
// gloox
#include <gloox/receipt.h>
// Kaidan
#include "MessageModel.h"
#include "Notifications.h"

MessageHandler::MessageHandler(gloox::Client *client, MessageModel *model)
{
	messageModel = model;
	this->client = client;
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::handleMessage(const gloox::Message &message, gloox::MessageSession *session)
{
	QString body = QString::fromStdString(message.body());
	
	if (body.size() > 0) {
		//
		// add the message to the db
		//

		// author is only the 'bare' JID: e.g. 'albert@einstein.ch'
		const QString author = QString::fromStdString(message.from().bare());
		const QString author_resource = QString::fromStdString(message.from().resource());
		const QString recipient = QString::fromStdString(message.to().bare());
		const QString recipient_resource = QString::fromStdString(message.to().resource());
		// TODO: XEP-0203: Delayed Delivery's timestamps
		QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
		
		const QString msgId = QString::fromStdString(message.id());

		messageModel->addMessage(&author, &recipient, &timestamp, &body, &msgId,
								 false, &author_resource, &recipient_resource);

		// send a new notification | TODO: Resolve nickname from JID
		Notifications::sendMessageNotification(message.from().full(), body.toStdString());
	}

	// XEP-0184: Message Delivery Receipts
	// try to get a possible delivery receipt
	gloox::Receipt *receipt = (gloox::Receipt*) message.findExtension<gloox::Receipt>(
		gloox::ExtReceipt);

	if (receipt) {
		// get the type of the receipt
		gloox::Receipt::ReceiptType receiptType = receipt->rcpt();
		if (receiptType == gloox::Receipt::Request) {
			// send the asked confirmation, that the message has been arrived
			// new message to the author of the request
			gloox::Message receiptMessage(gloox::Message::Chat, message.from());

			// add the receipt extension containing the request's message id
			gloox::Receipt *receiptPayload = new gloox::Receipt(gloox::Receipt::Received,
																message.id());
			receiptMessage.addExtension(receiptPayload);

			// send the receipt message
			client->send(receiptMessage);
		} else if (receiptType == gloox::Receipt::Received) {
			// Delivery Receipt Received -> mark message as read in db
			messageModel->setMessageAsDelivered(QString::fromStdString(receipt->id()));
		}
	}
}

void MessageHandler::sendMessage(QString *fromJid, QString *toJid, QString *body)
{
	// create a new message
	gloox::Message message(gloox::Message::Chat, gloox::JID(toJid->toStdString()),
						   body->toStdString());

	// add the message to the database
	const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
	const QString id = QString::fromStdString(message.id());

	messageModel->addMessage(fromJid, toJid, &timestamp, body, &id, true);

	// XEP-0184: Message Delivery Receipts
	// request a delivery receipt from the other client
	gloox::Receipt *receiptPayload = new gloox::Receipt(gloox::Receipt::Request);
	message.addExtension(receiptPayload);

	// send the message
	client->send(message);
}
