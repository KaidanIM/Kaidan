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

#include "Kaidan.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QStandardPaths>
#include <QCoreApplication>
// Kaidan
#include "AvatarFileStorage.h"
#include "RosterModel.h"
#include "MessageModel.h"
#include "Database.h"

Kaidan::Kaidan(QGuiApplication *app, QObject *parent) : QObject(parent)
{
	//
	// Database and components
	//

	// setup database
	database = new Database();
	database->openDatabase();
	if (database->needToConvert())
		database->convertDatabase();

	// setup components
	messageModel = new MessageModel(database->getDatabase(), this);
	rosterModel = new RosterModel(database->getDatabase(), this);
	avatarStorage = new AvatarFileStorage(this);
	// Connect the avatar changed signal of the avatarStorage with the NOTIFY signal
	// of the Q_PROPERTY for the avatar storage (so all avatars are updated in QML)
	connect(avatarStorage, &AvatarFileStorage::avatarIdsChanged,
	        this, &Kaidan::avatarStorageChanged);

	//
	// Load settings data
	//

	// init settings (-> "kaidan/kaidan.conf")
	settings = new QSettings(QString(APPLICATION_NAME), QString(APPLICATION_NAME));

	creds.jid = settings->value("auth/jid").toString();
	creds.jidResource = settings->value("auth/resource").toString();
	creds.password = QString(QByteArray::fromBase64(settings->value("auth/password").toString().toUtf8()));
	// use Kaidan as resource, if no set
	if (creds.jidResource == "")
		setJidResource(QString(APPLICATION_NAME));

	creds.isFirstTry = false;

	// create new client and start thread's main loop (won't connect until requested)
	client = new ClientThread(rosterModel, messageModel, avatarStorage, creds,
	                          settings, app);
	client->start();
	
	connect(client, &ClientThread::connectionStateChanged, [=](ConnectionState state) {
		emit this->connectionStateChanged((quint8) state);
	});
	connect(client, &ClientThread::disconnReasonChanged, [=](DisconnReason reason) {
		emit this->disconnReasonChanged((quint8) reason);
	});
	connect(client, &ClientThread::newCredentialsNeeded, this, &Kaidan::newCredentialsNeeded);
	connect(client, &ClientThread::logInWorked, this, &Kaidan::logInWorked);
}

Kaidan::~Kaidan()
{
	delete client;
	delete rosterModel;
	delete messageModel;
	delete database;
	delete avatarStorage;
	delete settings;
}

void Kaidan::start()
{
	if (creds.jid.isEmpty() || creds.password.isEmpty())
		emit newCredentialsNeeded();
	else
		mainConnect();
}

bool Kaidan::mainConnect()
{
	if (client->isConnected()) {
		qWarning() << "[main] Tried to connect, even if still connected!"
		           << "Requesting disconnect.";
		emit client->disconnectRequested();
		return false;
	}

	// TODO: check if jid is valid first
	client->setCredentials(creds);
	emit client->connectRequested();
	return true;
}

void Kaidan::mainDisconnect(bool openLogInPage)
{
	// disconnect the client if connected or connecting
	if (client->isConnected())
		emit client->disconnectRequested();

	// trigger the gui to open the log in page in case the was wanted:
	if (openLogInPage)
		emit newCredentialsNeeded();
}

void Kaidan::setJid(QString jid)
{
	creds.jid = jid;
	// credentials were modified -> first try
	creds.isFirstTry = true;
}

void Kaidan::setJidResource(QString jidResource)
{
	// JID resource won't influence the authentication, so we don't need
	// to set the first try flag and can save it.
	creds.jidResource = jidResource;
	settings->setValue("auth/resource", jidResource);
}

void Kaidan::setPassword(QString password)
{
	creds.password = password;
	// credentials were modified -> first try
	creds.isFirstTry = true;
}

void Kaidan::setChatPartner(QString chatPartner)
{
	// check if different
	if (this->chatPartner == chatPartner)
		return;

	// set the new chat partner
	this->chatPartner = chatPartner;

	// filter message for this chat partner
	messageModel->applyRecipientFilter(&chatPartner, &(creds.jid));
	client->setCurrentChatPartner(&chatPartner);

	emit chatPartnerChanged();
}

quint8 Kaidan::getConnectionState() const
{
	return (quint8) client->getConnectionState();
}

quint8 Kaidan::getDisconnReason() const
{
	// gloox::ConnectionError -> (Kaidan) Enums::DisconnReason
	return (quint8) client->getConnectionError();
}

void Kaidan::sendMessage(QString jid, QString message)
{
	if (client->isConnected()) {
		emit client->sendMessageRequested(jid, message);
	} else {
		emit passiveNotificationRequested(tr("Could not send message, because not being connected."));
		qWarning() << "[main] Could not send message, because not being connected.";
	}
}

void Kaidan::addContact(QString jid, QString nick)
{
	if (client->isConnected()) {
		emit client->addContactRequested(jid, nick);
	} else {
		emit passiveNotificationRequested(tr("Could not add contact, because not being connected."));
		qWarning() << "[main] Could not add contact, because not being connected.";
	}
}

void Kaidan::removeContact(QString jid)
{
	if (client->isConnected()) {
		emit client->removeContactRequested(jid);
	} else {
		emit passiveNotificationRequested(tr("Could not remove contact, because not being connected."));
		qWarning() << "[main] Could not remove contact, because not being connected.";
	}
}

QString Kaidan::getResourcePath(QString name) const
{
	// list of file paths where to search for the resource file
	QStringList pathList;
	// add relative path from binary (only works if installed)
	pathList << QCoreApplication::applicationDirPath() + QString("/../share/") + QString(APPLICATION_NAME);
	// get the standard app data locations for current platform
	pathList << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
#ifndef NDEBUG
#ifdef DEBUG_SOURCE_PATH
	// add source directory (only for debug builds)
	pathList << QString(DEBUG_SOURCE_PATH) + QString("/data");
#endif
#endif

	// search for file in directories
	for (int i = 0; i < pathList.size(); i++) {
		// open directory
		QDir directory(pathList.at(i));
		// look up the file
		if (directory.exists(name)) {
			// found the file, return the path
			return QString("file://") + directory.absoluteFilePath(name);
		}
	}

	// on Android, we want to fetch images from the application resources
	if (QFile::exists(":/" + name))
		return QString("qrc:/" + name);

	// no file found
	qWarning() << "[main] Could NOT find media file:" << name;
	return QString("");
}
