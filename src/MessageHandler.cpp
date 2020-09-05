/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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
#include "ClientWorker.h"
#include "Globals.h"
#include "Kaidan.h"
#include "MessageModel.h"
#include "MediaUtils.h"

MessageHandler::MessageHandler(ClientWorker *clientWorker, QXmppClient *client, QObject *parent)
	: QObject(parent),
	  m_clientWorker(clientWorker),
	  m_client(client)
{
	connect(client, &QXmppClient::messageReceived, this, &MessageHandler::handleMessage);
	connect(this, &MessageHandler::sendMessageRequested, this, &MessageHandler::sendMessage);
	connect(MessageModel::instance(), &MessageModel::sendCorrectedMessageRequested,
	        this, &MessageHandler::sendCorrectedMessage);
	connect(MessageModel::instance(), &MessageModel::sendChatStateRequested,
	        this, &MessageHandler::sendChatState);

	client->addExtension(&m_receiptManager);
	connect(&m_receiptManager, &QXmppMessageReceiptManager::messageDelivered,
		this, [=] (const QString&, const QString &id) {
		emit MessageModel::instance()->setMessageDeliveryStateRequested(id, Enums::DeliveryState::Delivered);
	});

	m_carbonManager = new QXmppCarbonManager();
	client->addExtension(m_carbonManager);

	// messages sent to our account (forwarded from another client)
	connect(m_carbonManager, &QXmppCarbonManager::messageReceived,
	        client, &QXmppClient::messageReceived);
	// messages sent from our account (but another client)
	connect(m_carbonManager, &QXmppCarbonManager::messageSent,
	        client, &QXmppClient::messageReceived);

	// carbons discovery
	auto *discoveryManager = client->findExtension<QXmppDiscoveryManager>();
	if (!discoveryManager)
		return;

	connect(discoveryManager, &QXmppDiscoveryManager::infoReceived,
	        this, &MessageHandler::handleDiscoInfo);

	connect(MessageModel::instance(), &MessageModel::pendingMessagesFetched,
			this, &MessageHandler::handlePendingMessages);
}

MessageHandler::~MessageHandler()
{
	delete m_carbonManager;
}

void MessageHandler::handleMessage(const QXmppMessage &msg)
{
	if (msg.type() == QXmppMessage::Error) {
		emit MessageModel::instance()->setMessageDeliveryStateRequested(
				msg.id(), Enums::DeliveryState::Error, msg.error().text());
		return;
	}

	if (msg.state() != QXmppMessage::State::None) {
		emit MessageModel::instance()->handleChatStateRequested(
				QXmppUtils::jidToBareJid(msg.from()), msg.state());
	}

	if (msg.body().isEmpty() && msg.outOfBandUrl().isEmpty())
		return;

	Message message;
	message.setFrom(QXmppUtils::jidToBareJid(msg.from()));
	message.setTo(QXmppUtils::jidToBareJid(msg.to()));
	message.setSentByMe(QXmppUtils::jidToBareJid(msg.from()) == m_client->configuration().jidBare());
	message.setId(msg.id());
	// don't use file sharing fallback bodys
	if (msg.body() != msg.outOfBandUrl())
		message.setBody(msg.body());
	message.setMediaType(MessageType::MessageText); // default to text message without media
	message.setIsSpoiler(msg.isSpoiler());
	message.setSpoilerHint(msg.spoilerHint());
	message.setOutOfBandUrl(msg.outOfBandUrl());

	// check if message contains a link and also check out of band url
	if (!parseMediaUri(message, msg.outOfBandUrl(), false)) {
		const QStringList bodyWords = message.body().split(u' ');
		for (const QString &word : bodyWords) {
			if (parseMediaUri(message, word, true))
				break;
		}
	}

	// get possible delay (timestamp)
	message.setStamp((msg.stamp().isNull() || !msg.stamp().isValid())
	                 ? QDateTime::currentDateTimeUtc()
	                 : msg.stamp().toUTC());

	// save the message to the database
	// in case of message correction, replace old message
	if (msg.replaceId().isEmpty()) {
		emit MessageModel::instance()->addMessageRequested(message);
	} else {
		message.setIsEdited(true);
		message.setId(QString());
		emit MessageModel::instance()->updateMessageRequested(msg.replaceId(), [=](Message &m) {
			// replace completely
			m = message;
		});
	}

	// Send a message notification

	// The contact can differ if the message is really from a contact or just
	// a forward of another of the user's clients.
	QString contactJid = message.sentByMe() ? message.to() : message.from();
	// resolve user-defined name of this JID
	QString contactName = m_client->findExtension<QXmppRosterManager>()->getRosterEntry(contactJid).name();
	if (contactName.isEmpty())
		contactName = contactJid;

	// Show a notification for the message in the following cases:
	//  * The message was not sent by the user from another resource and received via Message Carbons.
	//  * Notifications from the chat partner are not muted.
	//  * The corresponding chat is not opened while the application window is active.
	if (!message.sentByMe() &&
			!Kaidan::instance()->notificationsMuted(contactJid) &&
			(MessageModel::instance()->currentChatJid() != message.from() ||
			 !m_clientWorker->isApplicationWindowActive())) {
		emit m_clientWorker->showMessageNotificationRequested(contactJid, contactName, msg.body());
	}
}

void MessageHandler::sendMessage(const QString& toJid,
                                 const QString& body,
                                 bool isSpoiler,
                                 const QString& spoilerHint)
{
	Message msg;
	msg.setFrom(m_client->configuration().jidBare());
	msg.setTo(toJid);
	msg.setBody(body);
	msg.setId(QXmppUtils::generateStanzaHash());
	msg.setReceiptRequested(true);
	msg.setSentByMe(true);
	msg.setMediaType(MessageType::MessageText); // text message without media
	msg.setDeliveryState(Enums::DeliveryState::Pending);
	msg.setStamp(QDateTime::currentDateTimeUtc());
	msg.setIsSpoiler(isSpoiler);
	msg.setSpoilerHint(spoilerHint);

	// process links from the body
	const QStringList words = body.split(u' ');
	for (const auto &word : words) {
		if (parseMediaUri(msg, word, true))
			break;
	}

	emit MessageModel::instance()->addMessageRequested(msg);
	sendPendingMessage(msg);
}

void MessageHandler::sendChatState(const QString &toJid, const QXmppMessage::State state)
{
	QXmppMessage message;
	message.setTo(toJid);
	message.setState(state);
	m_client->sendPacket(message);
}

void MessageHandler::sendCorrectedMessage(const Message &msg)
{
	auto deliveryState = Enums::DeliveryState::Sent;
	QString errorText;
	if (!m_client->sendPacket(msg)) {
		// TODO store in the database only error codes, assign text messages right in the QML
		emit Kaidan::instance()->passiveNotificationRequested(
			tr("Message correction was not successful."));
		errorText = "Message correction was not successful.";
		deliveryState = Enums::DeliveryState::Error;
	}

	emit MessageModel::instance()->updateMessageRequested(msg.id(), [=](Message &localMessage) {
		localMessage.setDeliveryState(deliveryState);
		localMessage.setErrorText(errorText);
	});
}

void MessageHandler::handleDiscoInfo(const QXmppDiscoveryIq &info)
{
	if (info.from() != m_client->configuration().domain())
		return;
	// enable carbons, if feature found
	if (info.features().contains(NS_CARBONS))
		m_carbonManager->setCarbonsEnabled(true);
}

void MessageHandler::sendPendingMessage(const Message &message)
{
	if (m_client->state() == QXmppClient::ConnectedState) {
		bool success;
		// if the message is a pending edition of the existing in the history message
		// I need to send it with the most recent stamp
		// for that I'm gonna copy that message and update in the copy just the stamp
		if (message.isEdited()) {
			Message msg = message;
			msg.setStamp(QDateTime::currentDateTimeUtc());
			success = m_client->sendPacket(msg);
		} else {
			success = m_client->sendPacket(message);
		}

		if (success) {
			emit MessageModel::instance()->setMessageDeliveryStateRequested(
					message.id(), Enums::DeliveryState::Sent);
		}
		// TODO this "true" from sendPacket doesn't yet mean the message was successfully sent

		else {
			qWarning() << "[client] [MessageHandler] Could not send message, as a result of"
				<< "QXmppClient::sendPacket returned false.";

			// The error message of the message is saved untranslated. To make
			// translation work in the UI, the tr() call of the passive
			// notification must contain exactly the same string.
			emit Kaidan::instance()->passiveNotificationRequested(tr("Message could not be sent."));
			emit MessageModel::instance()->setMessageDeliveryStateRequested(
					message.id(), Enums::DeliveryState::Error, "Message could not be sent.");
		}
	}
}

bool MessageHandler::parseMediaUri(Message &message, const QString &uri, bool isBodyPart)
{
	if (!MediaUtils::isHttp(uri) && !MediaUtils::isGeoLocation(uri)) {
		return false;
	}

	// check message type by file name in link
	// This is hacky, but needed without SIMS or an additional HTTP request.
	// Also, this can be useful when a user manually posts an HTTP url.
	const QUrl url(uri);
	const QMimeType mimeType = MediaUtils::mimeType(url);
	const MessageType messageType = MediaUtils::messageType(mimeType);

	switch (messageType) {
	case MessageType::MessageText:
	case MessageType::MessageUnknown:
		break;
	case MessageType::MessageFile:
		// Random files could be anything and could also include any website. We
		// want to avoid random links to be recognized as 'file'. Intentionally
		// sent files should be displayed of course.
		if (isBodyPart)
			break;
		[[fallthrough]];
	case MessageType::MessageGeoLocation:
		message.setMediaLocation(url.toEncoded());
		[[fallthrough]];
	case MessageType::MessageImage:
	case MessageType::MessageAudio:
	case MessageType::MessageVideo:
	case MessageType::MessageDocument:
		message.setMediaType(messageType);
		message.setMediaContentType(mimeType.name());
		message.setOutOfBandUrl(url.toEncoded());
		return true;
	}

	return false;
}

void MessageHandler::handlePendingMessages(const QVector<Message> &messages)
{
	for (const Message &message : qAsConst(messages)) {
		sendPendingMessage(message);
	}
}
