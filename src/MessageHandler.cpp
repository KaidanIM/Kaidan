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
#include <QUrl>
// QXmpp
#include <QXmppCarbonManager.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppRosterManager.h>
#include <QXmppUtils.h>
// Kaidan
#include "ClientWorker.h"
#include "Globals.h"
#include "Kaidan.h"
#include "Database.h"
#include "Message.h"
#include "MessageDb.h"
#include "MessageModel.h"
#include "MediaUtils.h"

// Number of messages fetched at once when loading MAM backlog
constexpr int MAM_BACKLOG_FETCH_COUNT = 40;

MessageHandler::MessageHandler(ClientWorker *clientWorker, QXmppClient *client, QObject *parent)
	: QObject(parent),
	  m_clientWorker(clientWorker),
	  m_client(client),
	  m_carbonManager(new QXmppCarbonManager),
	  m_mamManager(new QXmppMamManager)
{
	connect(client, &QXmppClient::messageReceived, this, [=](const QXmppMessage &msg) {
		handleMessage(msg, MessageOrigin::Stream);
	});
	connect(this, &MessageHandler::sendMessageRequested, this, &MessageHandler::sendMessage);
	connect(MessageModel::instance(), &MessageModel::sendCorrectedMessageRequested,
	        this, &MessageHandler::sendCorrectedMessage);
	connect(MessageModel::instance(), &MessageModel::sendChatStateRequested,
	        this, &MessageHandler::sendChatState);

	connect(client, &QXmppClient::connected, this, &MessageHandler::handleConnected);
	connect(client, &QXmppClient::disconnected, this, &MessageHandler::handleDisonnected);
	connect(client->findExtension<QXmppRosterManager>(), &QXmppRosterManager::rosterReceived,
	        this, &MessageHandler::handleRosterReceived);
	connect(MessageDb::instance(), &MessageDb::lastMessageStampFetched,
	        this, &MessageHandler::handleLastMessageStampFetched);

	connect(&m_receiptManager, &QXmppMessageReceiptManager::messageDelivered,
		this, [=](const QString &, const QString &id) {
		emit MessageModel::instance()->updateMessageRequested(id, [](Message &msg) {
			msg.setDeliveryState(Enums::DeliveryState::Delivered);
			msg.setErrorText({});
		});
	});

	// messages sent to our account (forwarded from another client)
	connect(m_carbonManager, &QXmppCarbonManager::messageReceived,
	        client, &QXmppClient::messageReceived);
	// messages sent from our account (but another client)
	connect(m_carbonManager, &QXmppCarbonManager::messageSent,
	        client, &QXmppClient::messageReceived);

	connect(m_mamManager, &QXmppMamManager::archivedMessageReceived, this, &MessageHandler::handleArchiveMessage);
	connect(m_mamManager, &QXmppMamManager::resultsRecieved, this, &MessageHandler::handleArchiveResults);

	connect(this, &MessageHandler::retrieveBacklogMessagesRequested, this, &MessageHandler::retrieveBacklogMessages);

	client->addExtension(&m_receiptManager);
	client->addExtension(m_carbonManager);
	client->addExtension(m_mamManager);

	// carbons discovery
	auto *discoveryManager = client->findExtension<QXmppDiscoveryManager>();
	if (!discoveryManager)
		return;

	connect(discoveryManager, &QXmppDiscoveryManager::infoReceived,
	        this, &MessageHandler::handleDiscoInfo);

	connect(MessageModel::instance(), &MessageModel::pendingMessagesFetched,
			this, &MessageHandler::handlePendingMessages);

	// get last message stamp to retrieve all new messages from the server since then
	emit MessageDb::instance()->fetchLastMessageStampRequested();
}

MessageHandler::~MessageHandler()
{
	delete m_carbonManager;
	delete m_mamManager;
}

void MessageHandler::handleRosterReceived()
{
	// retrieve initial messages for each contact, if there is no last message locally
	if (m_lastMessageLoaded && m_lastMessageStamp.isNull())
		retrieveInitialMessages();
}

void MessageHandler::handleLastMessageStampFetched(const QDateTime &stamp)
{
	m_lastMessageStamp = stamp;
	m_lastMessageLoaded = true;

	// this is for the case that loading the last message took longer than connecting to
	// the server:

	// if already connected directly retrieve messages
	if (m_client->isConnected()) {
		// if there are no messages at all, load initial history,
		// otherwise load all missed messages since last online.
		if (stamp.isNull()) {
			// only start if roster was received already
			if (m_client->findExtension<QXmppRosterManager>()->isRosterReceived())
				retrieveInitialMessages();
		} else {
			retrieveCatchUpMessages(stamp);
		}
	}
}

void MessageHandler::handleMessage(const QXmppMessage &msg, MessageOrigin origin)
{
	if (msg.type() == QXmppMessage::Error) {
		emit MessageModel::instance()->updateMessageRequested(msg.id(), [errorText { msg.error().text() }](Message &msg) {
			msg.setDeliveryState(Enums::DeliveryState::Error);
			msg.setErrorText(errorText);
		});
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
	message.setStanzaId(msg.stanzaId());
	message.setOriginId(msg.originId());

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
		emit MessageModel::instance()->addMessageRequested(message, origin);
	} else {
		message.setIsEdited(true);
		message.setId(QString());
		emit MessageModel::instance()->updateMessageRequested(msg.replaceId(), [=](Message &m) {
			// replace completely
			m = message;
		});
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
	msg.setOriginId(msg.id());
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

	emit MessageModel::instance()->addMessageRequested(msg, MessageOrigin::UserInput);
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

void MessageHandler::handleConnected()
{
	// retrieve missed messages, if the last saved message has been loaded and exists
	if (m_lastMessageLoaded && !m_lastMessageStamp.isNull()) {
		retrieveCatchUpMessages(m_lastMessageStamp);
	}
}

void MessageHandler::handleDisonnected()
{
	// clear all running backlog queries
	std::for_each(m_runningBacklogQueryIds.constKeyValueBegin(),
				  m_runningBacklogQueryIds.constKeyValueEnd(),
				  [=](const std::pair<QString, BacklogQueryState> &pair) {
		emit MessageModel::instance()->mamBacklogRetrieved(
				m_client->configuration().jidBare(), pair.second.chatJid, pair.second.lastTimestamp, false);
	});
	m_runningBacklogQueryIds.clear();

	m_runningInitialMessageQueryIds.clear();
	m_runnningCatchUpQueryId.clear();
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
			emit MessageModel::instance()->updateMessageRequested(message.id(), [](Message &msg) {
				msg.setDeliveryState(Enums::DeliveryState::Sent);
				msg.setErrorText({});
			});
		}
		// TODO this "true" from sendPacket doesn't yet mean the message was successfully sent

		else {
			qWarning() << "[client] [MessageHandler] Could not send message, as a result of"
				<< "QXmppClient::sendPacket returned false.";

			// The error message of the message is saved untranslated. To make
			// translation work in the UI, the tr() call of the passive
			// notification must contain exactly the same string.
			emit Kaidan::instance()->passiveNotificationRequested(tr("Message could not be sent."));
			emit MessageModel::instance()->updateMessageRequested(message.id(), [](Message &msg) {
				msg.setDeliveryState(Enums::DeliveryState::Error);
				msg.setErrorText(QStringLiteral("Message could not be sent."));
			});
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

void MessageHandler::handleArchiveMessage(const QString &queryId,
                                          const QXmppMessage &message)
{
	if (queryId == m_runnningCatchUpQueryId) {
		handleMessage(message, MessageOrigin::MamCatchUp);
	} else if (m_runningInitialMessageQueryIds.contains(queryId)) {
		// TODO: request other message if this message is empty (e.g. no body)
		handleMessage(message, MessageOrigin::MamInitial);
	} else if (m_runningBacklogQueryIds.contains(queryId)) {
		handleMessage(message, MessageOrigin::MamBacklog);
		// update last stamp
		auto &lastTimestamp = m_runningBacklogQueryIds[queryId].lastTimestamp;
		if (lastTimestamp > message.stamp()) {
			lastTimestamp = message.stamp();
		}
	}
}

void MessageHandler::handleArchiveResults(const QString &queryId,
                                          const QXmppResultSetReply &,
                                          bool complete)
{
	if (queryId == m_runnningCatchUpQueryId) {
		m_runnningCatchUpQueryId.clear();
		emit Kaidan::instance()->database()->commitRequested();
		return;
	}

	if (m_runningInitialMessageQueryIds.contains(queryId)) {
		m_runningInitialMessageQueryIds.removeOne(queryId);

		if (m_runningInitialMessageQueryIds.isEmpty()) {
			emit Kaidan::instance()->database()->commitRequested();

			// so this won't be triggered again on reconnect
			m_lastMessageStamp = QDateTime::currentDateTimeUtc();
		}
		return;
	}

	if (m_runningBacklogQueryIds.contains(queryId)) {
		const auto state = m_runningBacklogQueryIds.take(queryId);
		emit MessageModel::instance()->mamBacklogRetrieved(m_client->configuration().jidBare(), state.chatJid, state.lastTimestamp, complete);
	}
}

void MessageHandler::retrieveInitialMessages()
{
	QXmppResultSetQuery queryLimit;
	// load only one message per user (the rest can be loaded when needed)
	queryLimit.setMax(1);
	// query last (newest) first
	queryLimit.setBefore("");

	const auto bareJids = m_client->findExtension<QXmppRosterManager>()->getRosterBareJids();
	if (bareJids.isEmpty()) {
		return;
	}

	m_runningInitialMessageQueryIds.clear();
	m_runningInitialMessageQueryIds.reserve(bareJids.size());

	for (const auto &jid : bareJids) {
		m_runningInitialMessageQueryIds.push_back(m_mamManager->retrieveArchivedMessages(
			QString(),
			QString(),
			jid,
			QDateTime(),
			QDateTime(),
			queryLimit
		));
	}

	emit Kaidan::instance()->database()->transactionRequested();
}

void MessageHandler::retrieveCatchUpMessages(const QDateTime &stamp)
{
	QXmppResultSetQuery queryLimit;
	// no limit
	queryLimit.setMax(-1);

	m_runnningCatchUpQueryId = m_mamManager->retrieveArchivedMessages({}, {}, {}, stamp, {}, queryLimit);

	emit Kaidan::instance()->database()->transactionRequested();
}

void MessageHandler::retrieveBacklogMessages(const QString &jid, const QDateTime &stamp)
{
	QXmppResultSetQuery queryLimit;
	queryLimit.setBefore("");
	queryLimit.setMax(MAM_BACKLOG_FETCH_COUNT);

	const auto id = m_mamManager->retrieveArchivedMessages({}, {}, jid, {}, stamp, queryLimit);
	m_runningBacklogQueryIds.insert(id, BacklogQueryState { jid, stamp });
}
