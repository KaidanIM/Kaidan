/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

#include "ClientThread.h"
// Kaidan
#include "ClientWorker.h"
#include "AvatarFileStorage.h"
#include "RosterManager.h"
#include "PresenceHandler.h"
#include "ServiceDiscoveryManager.h"
#include "Database.h"
#include "MessageSessionHandler.h"
#include "MessageHandler.h"
#include "VCardManager.h"
#include "XmlLogHandler.h"
// Qt
#include <QDebug>
#include <QMutexLocker>
#include <QTimer>
// gloox
#include <gloox/rostermanager.h>
#include <gloox/receipt.h>
#include <gloox/forward.h>
#include <gloox/carbons.h>
#include <gloox/vcardmanager.h>
#include <gloox/vcardupdate.h>

// package fetch interval in ms
static const unsigned int KAIDAN_CLIENT_LOOP_INTERVAL = 30;

ClientThread::ClientThread(RosterModel *rosterModel, MessageModel *messageModel,
                           AvatarFileStorage *avatarStorage, Credentials creds,
                           QSettings *settings, QObject *parent):
                           QThread(parent), rosterModel(rosterModel),
                           messageModel(messageModel), avatarStorage(avatarStorage),
                           creds(creds), settings(settings),
                           connState(ConnectionState::StateNone)
{
	// Set custom thread name
	setObjectName("XmppClient");

	client = new gloox::Client(gloox::JID(creds.jid.toStdString()),
	                           creds.password.toStdString());
	client->bindResource(creds.jidResource.toStdString()); // set resource / device name
	client->setTls(gloox::TLSRequired); // require encryption

	worker = new ClientWorker(client, this);
	connect(this, &ClientThread::connectRequested, worker, &ClientWorker::xmppConnect);
	connect(this, &ClientThread::disconnectRequested, worker, &ClientWorker::xmppDisconnect);
	connect(this, &ClientThread::stopWorkTimerRequested, worker, &ClientWorker::stopWorkTimer);

	// The constructor is executed in the main thread, so we still need to move
	// all QObject-based objects to the client thread.
	worker->moveToThread(this);
	workTimer.moveToThread(this);
}

ClientThread::~ClientThread()
{
	// This is being executed in the main thread, not the client thread itself

	// stop client loop timer from client thread
	emit stopWorkTimerRequested();

	exit(); // exit event loop
	wait(); // wait for run() to finish

	delete worker;
	delete messageSessionHandler;
	delete xmlLogHandler;
}

void ClientThread::run()
{
	//
	// Construct client and subclasses
	//

	// components
	messageSessionHandler = new MessageSessionHandler(client, messageModel, rosterModel);
	vCardManager = new VCardManager(client, avatarStorage, rosterModel);
	rosterManager = new RosterManager(client, rosterModel, vCardManager);
	presenceHandler = new PresenceHandler(client);
	serviceDiscoveryManager = new ServiceDiscoveryManager(client, client->disco());
	xmlLogHandler = new XmlLogHandler(client);

	// Register Stanza Extensions
	client->registerStanzaExtension(new gloox::Receipt(gloox::Receipt::Request));
	client->registerStanzaExtension(new gloox::DelayedDelivery(gloox::JID(), std::string("")));
	client->registerStanzaExtension(new gloox::Forward());
	client->registerStanzaExtension(new gloox::Carbons());
	client->registerStanzaExtension(new gloox::VCardUpdate());

	// connect slots
	connect(this, &ClientThread::sendMessageRequested,
	        messageSessionHandler->getMessageHandler(), &MessageHandler::sendMessage);
	connect(this, &ClientThread::addContactRequested,
	        rosterManager, &RosterManager::addContact);
	connect(this, &ClientThread::removeContactRequested,
	        rosterManager, &RosterManager::removeContact);

	// timed fetching of packages
	connect(&workTimer, &QTimer::timeout, worker, &ClientWorker::updateClient);
	workTimer.start(KAIDAN_CLIENT_LOOP_INTERVAL);

	// enter event loop
	qDebug() << "[client] Entering main loop...";
	exec();
}

void ClientThread::setCredentials(Credentials creds)
{
	QMutexLocker locker(&mutex); // => locking mutex in this function

	this->creds = creds;

	client->setUsername(gloox::JID(creds.jid.toStdString()).username());
	client->setServer(gloox::JID(creds.jid.toStdString()).server());
	client->setPassword(creds.password.toStdString());
	client->unbindResource(client->resource());
	client->bindResource(creds.jidResource.toStdString());
}

void ClientThread::setCurrentChatPartner(QString *jid)
{
	QMutexLocker locker(&mutex); // => locking mutex in this function
	messageSessionHandler->getMessageHandler()->setCurrentChatPartner(jid);
}

void ClientThread::setConnectionState(ConnectionState state)
{
	connState = state;
	emit connectionStateChanged(state);
}

void ClientThread::setConnectionError(gloox::ConnectionError error)
{
	connError = error;
	emit disconnReasonChanged((DisconnReason) error);
}
