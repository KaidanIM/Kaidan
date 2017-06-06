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
// Boost
#include <boost/bind.hpp>
// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/Client/MemoryStorages.h>
#include <Swiften/Crypto/PlatformCryptoProvider.h>
#include <Swiften/Queries/Responders/SoftwareVersionResponder.h>
// Kaidan
#include "RosterController.h"
#include "PresenceController.h"
#include "MessageController.h"
#include "MessageModel.h"
#include "VCardController.h"
#include "ServiceDiscoveryManager.h"

Kaidan::Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent) : QObject(parent)
{
	netFactories = networkFactories;
	connected = false;

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

	// create/update the full jid
	updateFullJid();

	// load controllers
	messageController = new MessageController(&jid);
	rosterController = new RosterController();
	presenceController = new PresenceController();
	vCardController = new VCardController();
	serviceDiscoveryManager = new ServiceDiscoveryManager();

	messageController->setRosterController(rosterController);

	storages = new Swift::MemoryStorages(Swift::PlatformCryptoProvider::create());
}

Kaidan::~Kaidan()
{
	if (connected) {
		client->disconnect();
		softwareVersionResponder->stop();
		delete tracer;
		delete softwareVersionResponder;
		delete messageController;
		delete client;
	}

	delete rosterController;
	delete presenceController;
	delete vCardController;
	delete settings;
	delete serviceDiscoveryManager;
}

void Kaidan::mainConnect()
{
	// Create a new XMPP client
	client = new Swift::Client(fullJid.toStdString(), password.toStdString(), netFactories, storages);

	// trust all certificates
	client->setAlwaysTrustCertificates();

	// event handling
	client->onConnected.connect(boost::bind(&Kaidan::handleConnected, this));
	client->onDisconnected.connect(boost::bind(&Kaidan::handleDisconnected, this));

	// Create XML tracer (console output of xmpp data)
	tracer = new Swift::ClientXMLTracer(client);

	// share kaidan version and sytem info
	QString systemInfo = QSysInfo::prettyProductName();
	softwareVersionResponder = new Swift::SoftwareVersionResponder(client->getIQRouter());
	softwareVersionResponder->setVersion(APPLICATION_DISPLAY_NAME, VERSION_STRING, systemInfo.toStdString());
	softwareVersionResponder->start();

	// set client in message, roster and presence controller
	messageController->setClient(client);
	rosterController->setClient(client);
	presenceController->setClient(client);
	vCardController->setClient(client);
	serviceDiscoveryManager->setClient(client);

	Swift::ClientOptions options;
	options.useStreamCompression = false;

	// .. and connect!
	client->connect(options);
}

// we don't want to close client without disconnection
void Kaidan::mainDisconnect()
{
	if (getConnectionState()) {
		client->disconnect();
	}
}

void Kaidan::handleConnected()
{
	// emit connected signal
	connected = true;
	emit connectionStateConnected();
	client->sendPresence(Swift::Presence::create("Send me a message"));
}

void Kaidan::handleDisconnected()
{
	connected = false;
	emit connectionStateDisconnected();
}

void Kaidan::updateFullJid()
{
	fullJid = jid + QString("/") + jidResource;
}

void Kaidan::setJid(QString jid_)
{
	jid = jid_; // set new jid for mainConnect
	settings->setValue("auth/jid", jid_); // save to settings
	updateFullJid();
}

void Kaidan::setJidResource(QString jidResource_)
{
	jidResource = jidResource_;
	settings->setValue("auth/resource", jidResource); // save jid resource
	updateFullJid(); // update the full jid (the resource part has changed)
}

void Kaidan::setPassword(QString password_)
{
	password = password_; // set new password for
	settings->setValue("auth/password", password_); // save to settings
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

// if no jid or password, return true
bool Kaidan::newLoginNeeded()
{
	return (jid == "") || (password == "");
}

RosterController* Kaidan::getRosterController()
{
	return rosterController;
}
MessageController* Kaidan::getMessageController()
{
	return messageController;
}
VCardController* Kaidan::getVCardController()
{
	return vCardController;
}
bool Kaidan::getConnectionState() const
{
	return connected;
}
QString Kaidan::getVersionString()
{
	return QString(VERSION_STRING);
}

QString Kaidan::getJid()
{
	return jid;
}
QString Kaidan::getJidResource()
{
	return jidResource;
}
QString Kaidan::getPassword()
{
	return password;
}
