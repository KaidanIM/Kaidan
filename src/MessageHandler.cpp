/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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
// Qt
#include <QDateTime>
#include <QMimeDatabase>
#include <QUrl>
// QXmpp
#include <QXmppCarbonManager.h>
#include <QXmppClient.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppRosterManager.h>
#include <QXmppUtils.h>
// Kaidan
#include "Kaidan.h"
#include "Message.h"
#include "MessageModel.h"
#include "Notifications.h"
#include "MediaUtils.h"

MessageHandler::MessageHandler(Kaidan *kaidan, QXmppClient *client, MessageModel *model,
                               QObject *parent)
	: QObject(parent), kaidan(kaidan), client(client), model(model)
{
	connect(client, &QXmppClient::messageReceived, this, &MessageHandler::handleMessage);
	connect(kaidan, &Kaidan::sendMessage, this, &MessageHandler::sendMessage);
	connect(kaidan, &Kaidan::correctMessage, this, &MessageHandler::correctMessage);

	client->addExtension(&receiptManager);
	connect(&receiptManager, &QXmppMessageReceiptManager::messageDelivered,
	        this, [=] (const QString&, const QString &id) {
		emit model->setMessageAsDeliveredRequested(id);
	});

	carbonManager = new QXmppCarbonManager();
	client->addExtension(carbonManager);

	// messages sent to our account (forwarded from another client)
	connect(carbonManager, &QXmppCarbonManager::messageReceived,
	        client, &QXmppClient::messageReceived);
	// messages sent from our account (but another client)
	connect(carbonManager, &QXmppCarbonManager::messageSent,
	        client, &QXmppClient::messageReceived);

	// carbons discovery
	auto *discoManager = client->findExtension<QXmppDiscoveryManager>();
	if (!discoManager)
		return;

	connect(discoManager, &QXmppDiscoveryManager::infoReceived,
	        this, &MessageHandler::handleDiscoInfo);
}

MessageHandler::~MessageHandler()
{
	delete carbonManager;
}

void MessageHandler::handleMessage(const QXmppMessage &msg)
{
    if (msg.body().isEmpty() || msg.type() == QXmppMessage::Error)
		return;

	Message message;
	message.setFrom(QXmppUtils::jidToBareJid(msg.from()));
	message.setTo(QXmppUtils::jidToBareJid(msg.to()));
	message.setSentByMe(msg.from() == client->configuration().jidBare());
	message.setId(msg.id());
	message.setBody(msg.body());
	message.setMediaType(MessageType::MessageText); // default to text message without media
#if (QXMPP_VERSION) >= QT_VERSION_CHECK(1, 1, 0)
	message.setIsSpoiler(msg.isSpoiler());
	message.setSpoilerHint(msg.spoilerHint());
#else
	for (const QXmppElement &extension : msg.extensions()) {
		if (extension.tagName() == "spoiler" &&
		    extension.attribute("xmlns") == NS_SPOILERS) {
			message.setIsSpoiler(true);
			message.setSpoilerHint(extension.value());
			break;
		}
	}
#endif

	// check if message contains a link and also check out of band url
	QStringList bodyWords = message.body().split(" ");
	bodyWords.prepend(msg.outOfBandUrl());

	for (const QString &word : qAsConst(bodyWords)) {
		if (!MediaUtils::isHttp(word) && !MediaUtils::isGeoLocation(word)) {
			continue;
		}

		// check message type by file name in link
		// This is hacky, but needed without SIMS or an additional HTTP request.
		// Also, this can be useful when a user manually posts an HTTP url.
		const QUrl url(word);
		const QMimeType mimeType = MediaUtils::mimeType(url);
		const MessageType messageType = MediaUtils::messageType(mimeType);

		switch (messageType) {
		case MessageType::MessageImage:
		case MessageType::MessageAudio:
		case MessageType::MessageVideo:
		case MessageType::MessageDocument:
		case MessageType::MessageFile:
		case MessageType::MessageGeoLocation:
			message.setMediaType(messageType);
			if (messageType == MessageType::MessageGeoLocation) {
				message.setMediaLocation(url.toEncoded());
			}
			message.setMediaContentType(mimeType.name());
			message.setOutOfBandUrl(url.toEncoded());
			break;
		case MessageType::MessageText:
		case MessageType::MessageUnknown:
			continue;
		}

		break; // we can only handle one link
	}

	// get possible delay (timestamp)
	message.setStamp((msg.stamp().isNull() || !msg.stamp().isValid())
	                 ? QDateTime::currentDateTimeUtc()
	                 : msg.stamp().toUTC());

	// save the message to the database
	// in case of message correction, replace old message
	if (msg.replaceId().isEmpty()) {
		emit model->addMessageRequested(message);
	} else {
		message.setIsEdited(true);
		message.setId(QString());
		emit model->updateMessageRequested(msg.replaceId(), [=] (Message &m) {
			// replace completely
			m = message;
		});
	}

	// Send a message notification

	// The contact can differ if the message is really from a contact or just
	// a forward of another of the user's clients.
	QString contactJid = message.sentByMe() ? message.to() : message.from();
	// resolve user-defined name of this JID
	QString contactName = client->findExtension<QXmppRosterManager>()->getRosterEntry(contactJid).name();
	if (contactName.isEmpty())
		contactName = contactJid;

	if (!message.sentByMe())
		Notifications::sendMessageNotification(contactJid, contactName, msg.body());

	// TODO: Move back following call to RosterManager::handleMessage when spoiler
	// messages are implemented in QXmpp
	const QString lastMessage =
		message.isSpoiler() ? message.spoilerHint().isEmpty() ? tr("Spoiler")
								      : message.spoilerHint()
				    : msg.body();

	emit kaidan->getRosterModel()->setLastMessageRequested(contactJid, lastMessage);
}

void MessageHandler::sendMessage(const QString& toJid,
                                 const QString& body,
                                 bool isSpoiler,
                                 const QString& spoilerHint)
{
	// TODO: Add offline message cache and send when connnected again
	if (client->state() != QXmppClient::ConnectedState) {
		emit kaidan->passiveNotificationRequested(
			tr("Could not send message, as a result of not being connected.")
		);
		qWarning() << "[client] [MessageHandler] Could not send message, as a result of "
		              "not being connected.";
		return;
	}

	Message msg;
	msg.setFrom(client->configuration().jidBare());
	msg.setTo(toJid);
	msg.setBody(body);
	msg.setId(QXmppUtils::generateStanzaHash(28));
	msg.setReceiptRequested(true);
	msg.setSentByMe(true);
	msg.setMediaType(MessageType::MessageText); // text message without media
	msg.setStamp(QDateTime::currentDateTimeUtc());
	if (isSpoiler) {
		msg.setIsSpoiler(isSpoiler);
		msg.setSpoilerHint(spoilerHint);

		// parsing/serialization of spoilers isn't implemented in QXmpp
		QXmppElementList extensions = msg.extensions();
		QXmppElement spoiler = QXmppElement();
		spoiler.setTagName("spoiler");
		spoiler.setValue(msg.spoilerHint());
		spoiler.setAttribute("xmlns", NS_SPOILERS);
		extensions.append(spoiler);
		msg.setExtensions(extensions);
	} else if (MediaUtils::isGeoLocation(msg.body())) {
		const QUrl url(msg.body());
		const QMimeType mimeType = MediaUtils::mimeType(url);
		const MessageType messageType = MediaUtils::messageType(mimeType);
		msg.setMediaType(messageType);
		msg.setMediaLocation(msg.body());
		msg.setMediaContentType(mimeType.name());
		msg.setOutOfBandUrl(msg.body());
	}

	emit model->addMessageRequested(msg);

	if (client->sendPacket(static_cast<QXmppMessage>(msg)))
		emit model->setMessageAsSentRequested(msg.id());
	else
	        emit kaidan->passiveNotificationRequested(tr("Message could not be sent."));
	// TODO: handle error
}

void MessageHandler::correctMessage(const QString& toJid,
                                    const QString& msgId,
                                    const QString& body)
{
	// TODO: load old message from model and put everything into the new message
	//       instead of only the new body

	// TODO: Add offline message cache and send when connnected again
	if (client->state() != QXmppClient::ConnectedState) {
		emit kaidan->passiveNotificationRequested(
			tr("Could not correct message, as a result of not being connected.")
		);
		qWarning() << "[client] [MessageHandler] Could not correct message, as a result of "
		              "not being connected.";
		return;
	}

	Message msg;
	msg.setFrom(client->configuration().jidBare());
	msg.setTo(toJid);
	msg.setId(QXmppUtils::generateStanzaHash(28));
	msg.setBody(body);
	msg.setReceiptRequested(true);
	msg.setSentByMe(true);
	msg.setMediaType(MessageType::MessageText); // text message without media
	msg.setIsEdited(true);
	msg.setReplaceId(msgId);

	emit model->updateMessageRequested(msgId, [=] (Message &msg) {
		msg.setBody(body);
	});
	if (client->sendPacket(msg))
		emit model->setMessageAsSentRequested(msg.id());
	else
		emit kaidan->passiveNotificationRequested(
	                        tr("Message correction was not successful."));
}

void MessageHandler::handleDiscoInfo(const QXmppDiscoveryIq &info)
{
	if (info.from() != client->configuration().domain())
		return;
	// enable carbons, if feature found
	if (info.features().contains(NS_CARBONS))
		carbonManager->setCarbonsEnabled(true);
}
