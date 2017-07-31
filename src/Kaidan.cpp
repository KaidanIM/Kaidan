/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
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

#include "Kaidan.h"

#include <iostream>
// Qt
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QStandardPaths>
#include <QTimer>
// gloox
#include <gloox/rostermanager.h>
#include <gloox/receipt.h>
#include <gloox/forward.h>
#include <gloox/carbons.h>
// Kaidan
#include "RosterModel.h"
#include "MessageModel.h"

Kaidan::Kaidan(QObject *parent) : QObject(parent)
{
	connected = false;

	// setup database
	database = new Database();
	database->openDatabase();
	if (database->needToConvert()) database->convertDatabase();

	// setup components
	messageModel = new MessageModel(database->getDatabase());
	rosterModel = new RosterModel(database->getDatabase());
	xmlLogHandler = new XmlLogHandler();

	//
	// Load settings data
	//

	// init settings (-> "kaidan/kaidan.conf")
	settings = new QSettings(QString(APPLICATION_NAME), QString(APPLICATION_NAME));

	// get JID from settings
	jid = settings->value("auth/jid").toString();
	// get JID resource from settings
	jidResource = settings->value("auth/resource").toString();
	// get password from settings
	password = settings->value("auth/password").toString();

	// use Kaidan as resource, if no set
	if (jidResource == "") {
		jidResource = QString(APPLICATION_NAME);
		settings->setValue("auth/resource", jidResource);
	}
}

Kaidan::~Kaidan()
{
	if (connected) {
		client->disconnect();
		delete client;
	}

	delete serviceDiscoveryManager;
	delete messageSessionHandler;
	delete rosterManager;
	delete settings;
}

void Kaidan::mainConnect()
{
	// Create a new XMPP client
	client = new gloox::Client(gloox::JID(jid.toStdString()), password.toStdString());
	// require encryption
	client->setTls(gloox::TLSPolicy::TLSRequired);
	// set the JID resource
	client->setResource(jidResource.toStdString());

	// Connection listener
	client->registerConnectionListener(this);

	// Message receiving/sending
	messageSessionHandler = new MessageSessionHandler(client, messageModel, rosterModel);
	client->registerMessageSessionHandler(messageSessionHandler);

	// Roster
	rosterManager = new RosterManager(rosterModel, client);

	// Presence Handler
	presenceHandler = new PresenceHandler(client);
	client->registerPresenceHandler(presenceHandler);

	// Service Discovery
	serviceDiscoveryManager = new ServiceDiscoveryManager(client, client->disco());

	// Register Stanza Extensions
	client->registerStanzaExtension(new gloox::Receipt(gloox::Receipt::Request));
	client->registerStanzaExtension(new gloox::Receipt(gloox::Receipt::Received));
	client->registerStanzaExtension(new gloox::DelayedDelivery(gloox::JID(), std::string("")));
	client->registerStanzaExtension(new gloox::Forward());
	client->registerStanzaExtension(new gloox::Carbons());

	// Logging
	client->logInstance().registerLogHandler(gloox::LogLevelDebug,
		gloox::LogAreaXmlIncoming | gloox::LogAreaXmlOutgoing, xmlLogHandler);

	client->connect(false);

	// every 100 ms: fetch new packages from the socket
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateClient()));
	timer->start(100);
}

void Kaidan::mainDisconnect()
{
	if (connected) {
		client->disconnect();
	}
}

void Kaidan::onConnect()
{
	qDebug() << "[Connection] Connected successfully to server";
	// emit connected signal
	connected = true;
	emit connectionStateConnected();
}

void Kaidan::onDisconnect(gloox::ConnectionError error)
{
	qDebug() << "[Connection] Connection failed or disconnected" << error;
	connected = false;
	emit connectionStateDisconnected();
}

bool Kaidan::onTLSConnect(const gloox::CertInfo &info)
{
	// accept certificate
	qDebug() << "[Connection] Automatically accepting TLS certificate";
	return true;
}

void Kaidan::updateClient()
{
	// parse new incoming network packages; wait 0 ms for new ones
	client->recv(0);
}

bool Kaidan::newLoginNeeded()
{
	return (jid == "") || (password == "");
}

QString Kaidan::getJid()
{
	return jid;
}

void Kaidan::setJid(QString jid_)
{
	jid = jid_; // set new jid for mainConnect
	settings->setValue("auth/jid", jid_); // save to settings
}

QString Kaidan::getJidResource()
{
	return jidResource;
}

void Kaidan::setJidResource(QString jidResource_)
{
	jidResource = jidResource_;
	settings->setValue("auth/resource", jidResource); // save jid resource
}

QString Kaidan::getPassword()
{
	return password;
}

void Kaidan::setPassword(QString password_)
{
	password = password_; // set new password for
	settings->setValue("auth/password", password_); // save to settings
}

QString Kaidan::getChatPartner()
{
	return chatPartner;
}

void Kaidan::setChatPartner(QString chatPartner)
{
	// check if different
	if (this->chatPartner == chatPartner)
		return;

	// set the new chat partner
	this->chatPartner = chatPartner;

	// filter message for this chat partner
	messageModel->applyRecipientFilter(&chatPartner, &jid);
	messageSessionHandler->getMessageHandler()->setCurrentChatPartner(&chatPartner);

	emit chatPartnerChanged();
}

void Kaidan::sendMessage(QString jid, QString message)
{
	if (connected) {
		messageSessionHandler->getMessageHandler()->sendMessage(&(this->jid), &jid, &message);
	}
}

void Kaidan::addContact(QString jid, QString nick)
{
	if (connected) {
		rosterManager->addContact(jid, nick);
	}
}

void Kaidan::removeContact(QString jid)
{
	if (connected) {
		rosterManager->removeContact(jid);
	}
}

QString Kaidan::getResourcePath(QString name_)
{
	// get the standard app data locations for current platform
	QStringList pathList = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
	pathList << QString(DEBUG_SOURCE_PATH) + QString("/data"); // append debug directory

	// search for file in directories
	for (int i = 0; i < pathList.size(); i++) {
		// open directory
		QDir directory(pathList.at(i));
		// look up the file
		if (directory.exists(name_)) {
			// found the file, return the path
			return QString("file://") + directory.absoluteFilePath(name_);
		}
	}

	// no file found
	qWarning() << "Could NOT find media file:" << name_;
	return QString("");
}

RosterModel* Kaidan::getRosterModel()
{
	return rosterModel;
}

MessageModel* Kaidan::getMessageModel()
{
	return messageModel;
}

bool Kaidan::getConnectionState() const
{
	return connected;
}

QString Kaidan::getVersionString()
{
	return QString(VERSION_STRING);
}
