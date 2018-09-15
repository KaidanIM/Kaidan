/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MessageHandler.h"
// Std
#include <iostream>
// Qt
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QMimeDatabase>
#include <QMimeType>
// gloox
#include <gloox/client.h>
#include <gloox/message.h>
#include <gloox/messagesession.h>
#include <gloox/receipt.h>
#include <gloox/carbons.h>
#include "gloox-extensions/gloox-extensions.h"
#include "gloox-extensions/reference.h"
#include "gloox-extensions/sims.h"
#include "gloox-extensions/jinglefile.h"
#include "gloox-extensions/bitsofbinarydata.h"
// Kaidan
#include "MessageModel.h"
#include "Notifications.h"

QDateTime stringToQDateTime(std::string stamp)
{
	QString qStamp = QString::fromStdString(stamp);
	QDateTime dateTime;

	if (qStamp.contains('.'))
		dateTime = QDateTime::fromString(qStamp, Qt::ISODateWithMs);
	else
		dateTime = QDateTime::fromString(qStamp, Qt::ISODate);

	if (!dateTime.isValid())
		return QDateTime::currentDateTime().toUTC();

	// XMPP timestamps are always in UTC
	// also read it as such if 'Z' is missing in ISO timestamp
	dateTime.setTimeSpec(Qt::UTC);
	return dateTime;
}

MessageHandler::MessageHandler(gloox::Client *client, MessageModel *messageModel,
                               RosterModel *rosterModel, QObject *parent):
                               QObject(parent), client(client),
                               messageModel(messageModel), rosterModel(rosterModel)
{
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::setChatPartner(QString chatPartner)
{
	this->chatPartner = chatPartner;

	resetUnreadMessagesForJid(this->chatPartner);
}

void MessageHandler::handleMessage(const gloox::Message &stanza, gloox::MessageSession *session)
{
	bool isCarbonMessage = false;

	// this should contain the real message (e.g. containing the body)
	gloox::Message *message = const_cast<gloox::Message*>(&stanza);
	// if the real message is in a message carbon extract it from there
	if (stanza.hasEmbeddedStanza()) {
		// get the possible carbon extension
		const gloox::Carbons *carbon = stanza.findExtension<const gloox::Carbons>(gloox::ExtCarbons);

		// if the extension exists and contains a message, use it as the real message
		if (carbon && carbon->embeddedStanza()) {
			isCarbonMessage = true;
			message = static_cast<gloox::Message*>(carbon->embeddedStanza());
		}
	}

	QString body = QString::fromStdString(message->body());

	if (!body.isEmpty()) {
		//
		// Extract information of the message
		//

		// author is only the 'bare' JID: e.g. 'albert@einstein.ch'
		MessageModel::Message msg;
		msg.author = QString::fromStdString(message->from().bare());
		msg.authorResource = QString::fromStdString(message->from().resource());
		msg.recipient = QString::fromStdString(message->to().bare());
		msg.recipientResource = QString::fromStdString(message->to().resource());
		msg.id = QString::fromStdString(message->id());
		msg.sentByMe = (msg.author == QString::fromStdString(client->jid().bare()));
		msg.message = body;
		msg.type = MessageType::MessageText; // only text, no media

		// handle media sharing (SIMS) content
		handleMediaSharing(const_cast<gloox::Message*>(message), &msg);

		//
		// If it is a delayed delivery (containing a timestamp), use its timestamp
		//

		const gloox::DelayedDelivery *delayedDelivery = message->when();
		if (delayedDelivery)
			msg.timestamp = stringToQDateTime(delayedDelivery->stamp())
			            .toString(Qt::ISODate);

		// fallback: use current time from local clock
		if (msg.timestamp.isEmpty())
			msg.timestamp = QDateTime::currentDateTime().toUTC()
			            .toString(Qt::ISODate);

		// add the message to the database
		emit messageModel->addMessageRequested(msg);

		//
		// Send a new notification | TODO: Resolve nickname from JID
		//

		if (!msg.sentByMe && !isCarbonMessage)
			Notifications::sendMessageNotification(message->from().full(), body.toStdString());

		//
		// Update contact sort (lastExchanged), last message and unread message count
		//

		// the contact can differ if the message is really from a contact or just
		// a forward of another of the user's clients
		const QString contactJid = msg.sentByMe ? msg.recipient : msg.author;

		// update the last message for this contact
		emit rosterModel->setLastMessageRequested(contactJid, body);

		// update the last exchanged for this contact
		updateLastExchangedOfJid(contactJid);

		// Increase unread message counter
		// don't add new unread message if chat is opened or we wrote the message
		if (!isCarbonMessage && chatPartner != contactJid && !msg.sentByMe)
			newUnreadMessageForJid(contactJid);
	}

	// XEP-0184: Message Delivery Receipts
	// try to handle a possible delivery receipt
	handleReceiptMessage(message, isCarbonMessage);
}

void MessageHandler::handleMediaSharing(const gloox::Message *message,
                                        MessageModel::Message *msg)
{
	//
	// Get media sharing (SIMS) information
	//

	const gloox::Reference *ref = message->findExtension
						<gloox::Reference>(gloox::EXT_REFERENCES);
	if (ref && ref->getEmbeddedSIMS()) {
		gloox::SIMS *sims = ref->getEmbeddedSIMS();
		gloox::StringList sources = sims->sources();
		for (auto &source : sources) {
			if (source.rfind("https://", 0) == 0 ||
				source.rfind("http://", 0) == 0) {
				msg->mediaUrl = QString::fromStdString(source);
				break;
			}
		}

		gloox::Jingle::File *file = sims->file();
		if (file && file->valid()) {
			msg->message = QString::fromStdString(file->desc());
			msg->mediaSize = file->size();
			msg->mediaContentType = QString::fromStdString(file->mediaType());
			msg->mediaLastModified = stringToQDateTime(file->date()).toMSecsSinceEpoch();
			QMimeType mimeType = QMimeDatabase().mimeTypeForName(msg->mediaContentType);
			msg->type = getMessageType(mimeType);
			for (gloox::Hash &hash : file->hashes())
				msg->mediaHashes.append(QString::fromStdString(hash.tag()->xml()));

			// extract thumbnail
			const gloox::Jingle::Thumb *thumb = file->thumb();
			if (thumb && thumb->valid()) {
				// check if uri is valid (it is a BoB content id [cid])
				std::string uri = thumb->uri();
				if (uri.rfind("cid:", 0) == 0) {
					const gloox::BitsOfBinaryData *thumbData = message->
						findExtension<gloox::BitsOfBinaryData>(gloox::EXT_BITSOFBINARY);

					// check if thumbnail uri matches the attached data uri
					if (thumbData && thumb->uri() == uri.substr(4, uri.length() - 4)) {
						// save media thumbnail
						msg->mediaThumb = QByteArray::fromBase64(
							QByteArray::fromStdString(thumbData->data())
						);
					}
				}
			}
		}
	}
}

void MessageHandler::handleReceiptMessage(const gloox::Message *message,
                                          bool isCarbonMessage)
{
	// check if message contains receipt
	gloox::Receipt *receipt = (gloox::Receipt*) message->findExtension<gloox::Receipt>(gloox::ExtReceipt);
	if (!receipt)
		return;

	// get the type of the receipt
	gloox::Receipt::ReceiptType receiptType = receipt->rcpt();

	if (receiptType == gloox::Receipt::Request && !isCarbonMessage) {
		// carbon messages won't be accepted to not send a receipt to own msgs
		// carbon msgs from other contacts should be processed by the first client

		// send the asked confirmation, that the message has been arrived
		// new message to the author of the request
		gloox::Message receiptMessage(gloox::Message::Chat, message->from());

		// add the receipt extension containing the request's message id
		gloox::Receipt *receiptPayload = new gloox::Receipt(gloox::Receipt::Received, message->id());
		receiptMessage.addExtension(receiptPayload);

		// send the receipt message
		client->send(receiptMessage);

	} else if (receiptType == gloox::Receipt::Received) {
		// Delivery Receipt Received -> mark message as read in db
		emit messageModel->setMessageAsDeliveredRequested(
			QString::fromStdString(receipt->id()));
	}
}

void MessageHandler::sendMessage(QString toJid, QString body)
{
	const std::string id = client->getID();

	addMessageToDb(toJid, body, QString::fromStdString(id), MessageType::MessageText);
	sendOnlyMessage(toJid, body, id);
}

void MessageHandler::sendOnlyMessage(QString &toJid, QString &body, const std::string &id)
{
	// create a new message
	gloox::Message message(gloox::Message::Chat, gloox::JID(toJid.toStdString()),
	                       body.toStdString());
	message.setID(id);

	// XEP-0184: Message Delivery Receipts
	// request a delivery receipt from the other client
	gloox::Receipt *receiptPayload = new gloox::Receipt(gloox::Receipt::Request);
	message.addExtension(receiptPayload);

	// send the message
	client->send(message);
}

void MessageHandler::addMessageToDb(QString &toJid, QString &body, QString id,
                                     MessageType type, QString mediaLocation)
{
	// add the message to the database
	MessageModel::Message msg;
	msg.timestamp = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
	msg.author = QString::fromStdString(client->jid().bare());
	msg.recipient = toJid;
	msg.message = body;
	msg.id = id;
	msg.sentByMe = true;
	msg.type = type;
	msg.mediaLocation = mediaLocation;

	emit messageModel->addMessageRequested(msg);

	// update the last message for this contact
	emit rosterModel->setLastMessageRequested(toJid, body);
	// update the last exchanged date
	updateLastExchangedOfJid(toJid);
}

void MessageHandler::updateLastExchangedOfJid(const QString &jid)
{
	QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
	emit rosterModel->setLastExchangedRequested(jid, dateTime);
}

void MessageHandler::newUnreadMessageForJid(const QString &jid)
{
	// add a new unread message to the contact
	emit rosterModel->newUnreadMessageRequested(jid);
}

void MessageHandler::resetUnreadMessagesForJid(const QString &jid)
{
	// reset the unread message count to 0
	emit rosterModel->setUnreadMessageCountRequested(jid, 0);
}

MessageType MessageHandler::getMessageType(QMimeType& type)
{
	if (type.inherits("image/jpeg") || type.inherits("image/png") ||
	    type.inherits("image/gif"))
		return MessageType::MessageImage;
	else if (type.inherits("audio/flac") || type.inherits("audio/mp4") ||
	         type.inherits("audio/ogg") || type.inherits("audio/wav") ||
	         type.inherits("audio/mpeg") || type.inherits("audio/webm"))
		return MessageType::MessageAudio;
	else if (type.inherits("video/mpeg") || type.inherits("video/x-msvideo") ||
	         type.inherits("video/quicktime") || type.inherits("video/mp4") ||
	         type.inherits("video/x-matroska"))
		return MessageType::MessageVideo;
	else if (type.inherits("text/plain"))
		return MessageType::MessageDocument;
	return MessageType::MessageFile;
}
