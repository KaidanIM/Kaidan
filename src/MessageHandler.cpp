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
#include <iostream>
// Qt
#include <QDateTime>
#include <QDebug>
#include <QString>
// gloox
#include <gloox/receipt.h>
#include <gloox/carbons.h>
// Kaidan
#include "MessageModel.h"
#include "Notifications.h"

QDateTime glooxStampToQDateTime(std::string stamp_)
{
	QString stamp = QString::fromStdString(stamp_);
	QDateTime dateTime;

	if (stamp.contains('Z')) {
		dateTime = QDateTime::fromString(stamp, "yyyy-MM-ddThh:mm:ss.zzzZ");
	} else if (stamp.contains('-') && stamp.contains(':') && stamp.contains('.')) {
		dateTime = QDateTime::fromString(stamp, "yyyy-MM-ddThh:mm:ss.zzz");
	} else if (stamp.contains('-') && stamp.contains(':')) {
		dateTime = QDateTime::fromString(stamp, "yyyy-MM-ddThh:mm:ss");
	} else if (stamp.contains(':') && stamp.contains('T')) {
		dateTime = QDateTime::fromString(stamp, "yyyyMMddThh:mm:ss");
	} else {
		dateTime = QDateTime::fromString(stamp, "hh:mm");
	}

	if (!dateTime.isValid())
		return QDateTime();

	dateTime.setTimeSpec(Qt::UTC);
	return dateTime;
}

MessageHandler::MessageHandler(gloox::Client *client, MessageModel *messageModel,
	RosterModel *rosterModel)
{
	this->messageModel = messageModel;
	this->rosterModel = rosterModel;
	this->client = client;
	this->chatPartner = QString("");
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::setCurrentChatPartner(QString *chatPartner)
{
	this->chatPartner = *chatPartner;

	resetUnreadMessagesForJid(chatPartner);
}

void MessageHandler::handleMessage(const gloox::Message &stanza, gloox::MessageSession *session)
{
	// this should contain the real message (e.g. containing the body)
	gloox::Message *message = const_cast<gloox::Message*>(&stanza);
	// if the real message is in a message carbon extract it from there
	if (stanza.hasEmbeddedStanza()) {
		// get the possible carbon extension
		const gloox::Carbons *carbon = stanza.findExtension<const gloox::Carbons>(gloox::ExtCarbons);

		// if the extension exists and contains a message, use it as the real message
		if(carbon && carbon->embeddedStanza())
			message = static_cast<gloox::Message*>(carbon->embeddedStanza());
	}

	QString body = QString::fromStdString(message->body());

	if (body.size() > 0) {
		//
		// Extract information of the message
		//

		// author is only the 'bare' JID: e.g. 'albert@einstein.ch'
		const QString fromJid = QString::fromStdString(message->from().bare());
		const QString fromJidResource = QString::fromStdString(message->from().resource());
		const QString toJid = QString::fromStdString(message->to().bare());
		const QString toJidResource = QString::fromStdString(message->to().resource());
		const QString msgId = QString::fromStdString(message->id());
		const bool isSentByMe = fromJid == QString::fromStdString(client->jid().bare());
		QString timestamp;

		//
		// If it is a delayed delivery (containing a timestamp), use its timestamp
		//

		const gloox::DelayedDelivery *delayedDelivery = message->when();
		if (delayedDelivery)
			timestamp = glooxStampToQDateTime(delayedDelivery->stamp()).toString(Qt::ISODate);

		// fallback: use current time from local clock
		if (timestamp.isEmpty())
			timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

		// add the message to the database
		messageModel->addMessage(&fromJid, &toJid, &timestamp, &body, &msgId,
								 isSentByMe, &fromJidResource, &toJidResource);

		//
		// Send a new notification | TODO: Resolve nickname from JID
		//

		if (!isSentByMe)
			Notifications::sendMessageNotification(message->from().full(), body.toStdString());

		//
		// Update contact sort (lastExchanged), last message and unread message count
		//

		// the contact can differ if the message is really from a contact or just
		// a forward of another of the user's clients
		const QString contactJid = isSentByMe ? toJid : fromJid;

		// update the last message for this contact
		rosterModel->setLastMessageForJid(&contactJid, &body);

		// update the last exchanged for this contact
		updateLastExchangedOfJid(&contactJid);

		// Increase unread message counter
		// don't add new unread message if chat is opened or we wrote the message
		if (chatPartner != contactJid && !isSentByMe)
			newUnreadMessageForJid(&contactJid);
	}

	if (message->hasEmbeddedStanza()) {
		// XEP-0184: Message Delivery Receipts
		// try to get a possible delivery receipt
		gloox::Receipt *receipt = (gloox::Receipt*) message->findExtension<gloox::Receipt>(gloox::ExtReceipt);

		if (receipt) {
			// get the type of the receipt
			gloox::Receipt::ReceiptType receiptType = receipt->rcpt();
			if (receiptType == gloox::Receipt::Request) {
				// send the asked confirmation, that the message has been arrived
				// new message to the author of the request
				gloox::Message receiptMessage(gloox::Message::Chat, message->from());

				// add the receipt extension containing the request's message id
				gloox::Receipt *receiptPayload = new gloox::Receipt(gloox::Receipt::Received,
																	message->id());
				receiptMessage.addExtension(receiptPayload);

				// send the receipt message
				client->send(receiptMessage);
			} else if (receiptType == gloox::Receipt::Received) {
				// Delivery Receipt Received -> mark message as read in db
				messageModel->setMessageAsDelivered(QString::fromStdString(receipt->id()));
			}
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

	// update the last message for this contact
	rosterModel->setLastMessageForJid(toJid, body);
	// update the last exchanged date
	updateLastExchangedOfJid(toJid);
}

void MessageHandler::updateLastExchangedOfJid(const QString *jid)
{
	QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
	rosterModel->setLastExchangedOfJid(jid, &dateTime);
}

void MessageHandler::newUnreadMessageForJid(const QString *jid)
{
	// get the current unread message count
	int msgCount = rosterModel->getUnreadMessageCountOfJid(jid);
	// increase it by one
	msgCount++;
	// set the new increased count
	rosterModel->setUnreadMessageCountOfJid(jid, msgCount);
}

void MessageHandler::resetUnreadMessagesForJid(const QString *jid)
{
	rosterModel->setUnreadMessageCountOfJid(jid, 0);
}
