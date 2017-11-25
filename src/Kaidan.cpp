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
#include <QCoreApplication>
// gloox
#include <gloox/rostermanager.h>
#include <gloox/receipt.h>
#include <gloox/forward.h>
#include <gloox/carbons.h>
#include <gloox/vcardmanager.h>
#include <gloox/vcardupdate.h>
// Kaidan
#include "RosterModel.h"
#include "MessageModel.h"

Kaidan::Kaidan(QObject *parent) : QObject(parent)
{
	connected = false;
	isClientSetUp = false;

	//
	// Database and components
	//

	// setup database
	database = new Database();
	database->openDatabase();
	if (database->needToConvert())
		database->convertDatabase();

	// setup components
	messageModel = new MessageModel(database->getDatabase());
	rosterModel = new RosterModel(database->getDatabase());
	xmlLogHandler = new XmlLogHandler();
	avatarStorage = new AvatarFileStorage();
	// Connect the avatar changed signal of the avatarStorage with the NOTIFY signal
	// of the Q_PROPERTY for the avatar storage (so all avatars are updated in QML)
	connect(avatarStorage, &AvatarFileStorage::avatarIdsChanged, this, &Kaidan::avatarStorageChanged);

	// client package fetch timer
	packageFetchTimer = new QTimer(this);
	connect(packageFetchTimer, SIGNAL(timeout()), this, SLOT(updateClient()));

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
	if (jidResource == "")
		setJidResource(QString(APPLICATION_NAME));
}

Kaidan::~Kaidan()
{
	// main disconnect will only try to disconnect, if connected
	mainDisconnect();
	// this will delete the client and its components
	if (isClientSetUp)
		clientCleanUp();

	delete rosterModel;
	delete messageModel;
	delete database;
	delete avatarStorage;
	delete xmlLogHandler;
	delete packageFetchTimer;
	delete settings;
}

void Kaidan::mainConnect()
{
	// first delete everything from the last connection
	if (isClientSetUp)
		clientCleanUp();

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

	// VCardManager
	vCardManager = new VCardManager(client, avatarStorage, rosterModel);

	// Roster
	rosterManager = new RosterManager(client, rosterModel, vCardManager);

	// Presence Handler
	presenceHandler = new PresenceHandler(client);

	// Service Discovery
	serviceDiscoveryManager = new ServiceDiscoveryManager(client, client->disco());

	// Register Stanza Extensions
	client->registerStanzaExtension(new gloox::Receipt(gloox::Receipt::Request));
	client->registerStanzaExtension(new gloox::DelayedDelivery(gloox::JID(), std::string("")));
	client->registerStanzaExtension(new gloox::Forward());
	client->registerStanzaExtension(new gloox::Carbons());
	client->registerStanzaExtension(new gloox::VCardUpdate());

	// Logging
	client->logInstance().registerLogHandler(gloox::LogLevelDebug,
		gloox::LogAreaXmlIncoming | gloox::LogAreaXmlOutgoing, xmlLogHandler);

	client->connect(false);

	// every 100 ms: fetch new packages from the socket
	packageFetchTimer->start(100);
}

void Kaidan::mainDisconnect()
{
	if (connected) {
		client->disconnect();
		packageFetchTimer->stop();
	}
}

void Kaidan::clientCleanUp()
{
	delete serviceDiscoveryManager;
	delete presenceHandler;
	delete rosterManager;
	delete messageSessionHandler;
	delete vCardManager;
	delete client;
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
	// TODO: Add offline message cache
	if (connected)
		messageSessionHandler->getMessageHandler()->sendMessage(&(this->jid), &jid, &message);
}

void Kaidan::addContact(QString jid, QString nick)
{
	// TODO: Add an error notification/message if not connected
	if (connected)
		rosterManager->addContact(jid, nick);
}

void Kaidan::removeContact(QString jid)
{
	// TODO: Add an error notification/message if not connected
	if (connected)
		rosterManager->removeContact(jid);
}

QString Kaidan::getResourcePath(QString name_)
{
	// list of file paths where to search for the resource file
	QStringList pathList;
	// add relative path from binary (only works if installed)
	pathList << QCoreApplication::applicationDirPath() + QString("/../share/") + QString(APPLICATION_NAME);
	// get the standard app data locations for current platform
	pathList << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
#ifndef NDEBUG
	// add source directory (only for debug builds)
	pathList << QString(DEBUG_SOURCE_PATH) + QString("/data"); // append debug directory
#endif

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

AvatarFileStorage* Kaidan::getAvatarStorage()
{
	return avatarStorage;
}

bool Kaidan::getConnectionState() const
{
	return connected;
}

QString Kaidan::getVersionString()
{
	return QString(VERSION_STRING);
}

QString Kaidan::removeNewLinesFromString(QString input)
{
	return input.simplified();
}
