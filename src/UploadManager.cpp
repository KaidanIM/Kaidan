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

#include "UploadManager.h"
#include "Kaidan.h"
#include "MessageHandler.h"
#include "RosterManager.h"
#include "TransferCache.h"
// QXmpp
#include <QXmppUtils.h>
// Qt
#include <QMimeDatabase>
#include <QMimeType>
#include <QMutexLocker>
#include <QDateTime>
#include <QBuffer>
#include <QImage>
#include <QDebug>
#include <QFileInfo>

UploadManager::UploadManager(Kaidan *kaidan, QXmppClient *client, MessageModel *msgModel,
                             RosterManager* rosterManager, TransferCache* transfers,
                             QObject* parent)
	: QObject(parent), kaidan(kaidan), client(client), msgModel(msgModel),
	rosterManager(rosterManager), transfers(transfers)
{
	client->addExtension(&manager);

	connect(kaidan, &Kaidan::sendFile, this, &UploadManager::sendFile);

	connect(&manager, &QXmppUploadManager::serviceFoundChanged, [this]() {
		// needed because kaidan is in main thread
		QMetaObject::invokeMethod(this->kaidan, "setUploadServiceFound", Qt::QueuedConnection,
		                          Q_ARG(bool, manager.serviceFound()));
	});
	connect(&manager, &QXmppUploadManager::uploadSucceeded,
	        this, &UploadManager::handleUploadSucceeded);
	connect(&manager, &QXmppUploadManager::uploadFailed,
	        this, &UploadManager::handleUploadFailed);
}

void UploadManager::sendFile(QString jid, QString fileUrl, QString body)
{
	// TODO: Add offline media message cache and send when connnected again
	if (client->state() != QXmppClient::ConnectedState) {
		emit kaidan->passiveNotificationRequested(
			tr("Could not send file, as a result of not being connected.")
		);
		qWarning() << "[client] [UploadManager] Could not send file, as a result of "
		              "not being connected.";
		return;
	}

	qDebug() << "[client] [UploadManager] Adding upload for file:" << fileUrl;

	QFileInfo file(QUrl(fileUrl).toLocalFile());
	const QXmppHttpUpload* upload = manager.uploadFile(file);

	QMimeType mimeType = QMimeDatabase().mimeTypeForFile(file);
	const QString msgId = QXmppUtils::generateStanzaHash(48);

	auto *msg = new Message;
	msg->setFrom(client->configuration().jidBare());
	msg->setTo(jid);
	msg->setId(msgId);
	msg->setSentByMe(true);
	msg->setBody(body);
	msg->setMediaType(Message::mediaTypeFromMimeType(mimeType));
	msg->setStamp(QDateTime::currentDateTimeUtc());
	msg->setMediaSize(file.size());
	msg->setMediaContentType(mimeType.name());
	msg->setMediaLastModified(file.lastModified());
	msg->setMediaLocation(file.filePath());

	// cache message and upload
	emit transfers->addJobRequested(msgId, upload->bytesTotal());
	messages.insert(upload->id(), msg);

	emit msgModel->addMessageRequested(*msg);

	// update last message
	QString lastMessage = tr("File");
	if (!body.isEmpty())
		lastMessage = lastMessage.append(": ").append(body);

	rosterManager->handleSendMessage(jid, lastMessage);

	connect(upload, &QXmppHttpUpload::bytesSentChanged, this, [upload, this, msgId] () {
		emit transfers->setJobBytesSentRequested(msgId, upload->bytesSent());
	});
}

void UploadManager::handleUploadSucceeded(const QXmppHttpUpload *upload)
{
	qDebug() << "[client] [UploadManager] A file upload has succeeded. Now sending message.";

	Message *originalMsg = messages.value(upload->id());

	const QString oobUrl = upload->slot().getUrl().toEncoded();
	const QString body = originalMsg->body().isEmpty()
	                     ? oobUrl
	                     : originalMsg->body() + "\n" + oobUrl;

	emit msgModel->updateMessageRequested(originalMsg->id(), [=] (Message &msg) {
		msg.setOutOfBandUrl(oobUrl);
		msg.setBody(body);
	});

	// send message
	QXmppMessage m(originalMsg->from(), originalMsg->to(), body);
	m.setId(originalMsg->id());
	m.setReceiptRequested(true);
	m.setStamp(originalMsg->stamp());
	m.setOutOfBandUrl(upload->slot().getUrl().toEncoded());

	bool success = client->sendPacket(m);
	if (success)
		emit msgModel->setMessageAsSentRequested(originalMsg->id());
	// TODO: handle error

	messages.remove(upload->id());
	emit transfers->removeJobRequested(originalMsg->id());
}

void UploadManager::handleUploadFailed(const QXmppHttpUpload *upload)
{
	qDebug() << "[client] [UploadManager] A file upload has failed.";
	const QString &msgId = messages.value(upload->id())->id();
	messages.remove(upload->id());
	emit transfers->removeJobRequested(msgId);
}
