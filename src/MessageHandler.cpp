/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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
#include <QString>
#include <QUrl>
// QXmpp
#include <QXmppClient.h>
#include <QXmppUtils.h>
#include <QXmppRosterManager.h>
#include <QXmppDiscoveryManager.h>
#if QXMPP_VERSION >= 0x000904
#include <QXmppCarbonManager.h>
#endif
// Kaidan
#include "Kaidan.h"
#include "MessageModel.h"
#include "Notifications.h"

MessageHandler::MessageHandler(Kaidan *kaidan, QXmppClient *client, MessageModel *model,
                               QObject *parent)
	: QObject(parent), kaidan(kaidan), client(client), model(model)
{
	connect(client, &QXmppClient::messageReceived, this, &MessageHandler::handleMessage);
	connect(kaidan, &Kaidan::sendMessage, this, &MessageHandler::sendMessage);
	connect(kaidan, &Kaidan::correctMessage, this, &MessageHandler::correctMessage);

	client->addExtension(&receiptManager);
	connect(&receiptManager, &QXmppMessageReceiptManager::messageDelivered,
		[=] (const QString&, const QString &id) {
		emit model->setMessageAsDeliveredRequested(id);
	});

#if QXMPP_VERSION >= 0x000904 // QXmppCarbonManager was added in v0.9.4
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
#endif
}

MessageHandler::~MessageHandler()
{
#if QXMPP_VERSION >= 0x000904
	delete carbonManager;
#endif
}

void MessageHandler::handleMessage(const QXmppMessage &msg)
{
	bool isCarbonMessage = false;

	if (msg.body().isEmpty())
		return;

	MessageModel::Message entry;
	entry.author = QXmppUtils::jidToBareJid(msg.from());
	entry.recipient = QXmppUtils::jidToBareJid(msg.to());
	entry.id = msg.id();
	entry.sentByMe = (entry.author == client->configuration().jidBare());
	entry.message = msg.body();
	entry.type = MessageType::MessageText; // default to text message without media

	// check if message contains a link and also check out of band url
	QList<QString> bodyWords = msg.body().split(" ");
#if QXMPP_VERSION >= QT_VERSION_CHECK(1, 0, 0)
	bodyWords.prepend(msg.outOfBandUrl());
#endif
	for (const QString &word : bodyWords) {
		bool isLink = word.startsWith("https://") || word.startsWith("http://");
		if (!isLink)
			continue;

		// check message type by file name in link
		// This is hacky, but needed without SIMS or an additional HTTP request.
		// Also, this can be useful when a user manually posts an HTTP url.
		QUrl url(word);
		QList<QMimeType> mediaTypes = QMimeDatabase().mimeTypesForFileName(url.fileName());
		for (const QMimeType &type : mediaTypes) {
			MessageType mType = MessageModel::messageTypeFromMimeType(type);
			if (mType == MessageType::MessageImage ||
			    mType == MessageType::MessageAudio ||
			    mType == MessageType::MessageVideo ||
			    mType == MessageType::MessageDocument ||
			    mType == MessageType::MessageFile) {
				entry.type = mType;
				entry.mediaContentType = type.name();
				entry.mediaUrl = url.toEncoded();
				break;
			}
		}
		break; // we can only handle one link
	}

	// get possible delay (timestamp)
	QDateTime stamp = msg.stamp();
	if (!stamp.isValid() || stamp.isNull())
		stamp = QDateTime::currentDateTime();
	entry.timestamp = stamp.toUTC().toString(Qt::ISODate);

	// save the message to the database
#if QXMPP_VERSION >= QT_VERSION_CHECK(1, 0, 0)
	// in case of message correction, replace old message
	if (msg.replaceId().isEmpty()) {
		emit model->addMessageRequested(entry);
	} else {
		entry.edited = true;
		emit model->updateMessageRequested(msg.replaceId(), entry);
	}
#else
	// no message correction with old QXmpp
	emit model->addMessageRequested(entry);
#endif

	// Send a message notification
	//
	// The contact can differ if the message is really from a contact or just
	// a forward of another of the user's clients.
	QString contactJid = entry.sentByMe ? entry.recipient : entry.author;
	// resolve user-defined name of this JID
	QString contactName = client->rosterManager().getRosterEntry(contactJid).name();
	if (contactName.isEmpty())
		contactName = contactJid;

	if (!entry.sentByMe && !isCarbonMessage)
		Notifications::sendMessageNotification(contactName.toStdString(),
		                                       msg.body().toStdString());
}

void MessageHandler::sendMessage(QString toJid, QString body)
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

	MessageModel::Message msg;
	msg.author = client->configuration().jidBare();
	msg.recipient = toJid;
	msg.id = QXmppUtils::generateStanzaHash(48);
	msg.sentByMe = true;
	msg.message = body;
	msg.type = MessageType::MessageText; // text message without media
	msg.timestamp = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);

	emit model->addMessageRequested(msg);

	QXmppMessage m(msg.author, msg.recipient, body);
	m.setId(msg.id);
	m.setReceiptRequested(true);

	bool success = client->sendPacket(m);
	if (success)
		emit model->setMessageAsSentRequested(msg.id);
	// TODO: handle error
}

void MessageHandler::correctMessage(QString toJid, QString msgId, QString body)
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

	MessageModel::Message msg;
	msg.author = client->configuration().jidBare();
	msg.recipient = toJid;
	msg.id = QXmppUtils::generateStanzaHash(48);
	msg.sentByMe = true;
	msg.message = body;
	msg.type = MessageType::MessageText; // text message without media
	msg.edited = true;

	emit model->updateMessageRequested(msgId, msg);

	QXmppMessage m(msg.author, msg.recipient, body);
	m.setId(msg.id);
	m.setReceiptRequested(true);
#if QXMPP_VERSION >= QT_VERSION_CHECK(1, 0, 0)
	m.setReplaceId(msgId);
#endif

	bool success = client->sendPacket(m);
	if (success)
		emit model->setMessageAsSentRequested(msg.id);
	// TODO: handle error
}

void MessageHandler::handleDiscoInfo(const QXmppDiscoveryIq &info)
{
#if QXMPP_VERSION >= 0x000904
	if (info.from() != client->configuration().domain())
		return;
	// enable carbons, if feature found
	if (info.features().contains(QString("urn:xmpp:carbons:2")))
		carbonManager->setCarbonsEnabled(true);
#endif
}
