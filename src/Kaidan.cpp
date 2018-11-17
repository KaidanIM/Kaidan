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
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QSettings>
#include <QString>
#include <QStandardPaths>
#include <QUrl>
// QXmpp
#include <QXmppClient.h>
// Kaidan
#include "AvatarFileStorage.h"
#include "PresenceCache.h"
#include "RosterModel.h"
#include "MessageModel.h"
#include "Database.h"

Kaidan::Kaidan(QGuiApplication *app, bool enableLogging, QObject *parent) : QObject(parent)
{
	// Database setup
	database = new Database();
	database->openDatabase();
	if (database->needToConvert())
		database->convertDatabase();

	// Caching components
	caches = new ClientWorker::Caches(database, this);

	// Connect the avatar changed signal of the avatarStorage with the NOTIFY signal
	// of the Q_PROPERTY for the avatar storage (so all avatars are updated in QML)
	connect(caches->avatarStorage, &AvatarFileStorage::avatarIdsChanged,
	        this, &Kaidan::avatarStorageChanged);

	//
	// Load settings
	//

	creds.jid = caches->settings->value("auth/jid").toString();
	creds.jidResource = caches->settings->value("auth/resource").toString();
	creds.password = QString(QByteArray::fromBase64(caches->settings->value("auth/password")
	                 .toString().toUtf8()));
	// use Kaidan as resource, if no set
	if (creds.jidResource.isEmpty())
		setJidResource(QString(APPLICATION_NAME));
	creds.isFirstTry = false;

	//
	// Start ClientWorker on new thread
	//

	// create new thread for client connection
	cltThrd = new ClientThread();
	client = new ClientWorker(caches, this, enableLogging, app);
	client->setCredentials(creds);

	client->moveToThread(cltThrd);
	connect(cltThrd, &QThread::started, client, &ClientWorker::main);

	cltThrd->start();
}

Kaidan::~Kaidan()
{
	delete client;
	delete caches;
	delete database;
	delete cltThrd;
}

void Kaidan::start()
{
	if (creds.jid.isEmpty() || creds.password.isEmpty())
		emit newCredentialsNeeded();
	else
		mainConnect();
}

void Kaidan::mainConnect()
{
	if (connectionState != ConnectionState::StateDisconnected) {
		qWarning() << "[main] Tried to connect, even if still connected!"
		           << "Requesting disconnect.";
		emit client->disconnectRequested();
	}

	emit client->credentialsUpdated(creds);
	emit client->connectRequested();

	// update own JID to display correct messages
	caches->msgModel->setOwnJid(creds.jid);
}

void Kaidan::mainDisconnect(bool openLogInPage)
{
	// disconnect the client if connected or connecting
	if (connectionState != ConnectionState::StateDisconnected)
		emit client->disconnectRequested();

	if (openLogInPage) {
		// clear password
		caches->settings->remove("auth/password");
		setPassword(QString());
		// trigger log in page
		emit newCredentialsNeeded();
	}
}

void Kaidan::setConnectionState(QXmppClient::State state)
{
	this->connectionState = (ConnectionState) state;
	emit connectionStateChanged();

	// Open the possibly cached URI when connected.
	// This is needed because the XMPP URIs can't be opened when Kaidan is not connected.
	if (connectionState == ConnectionState::StateConnected && !openUriCache.isEmpty()) {
		// delay is needed because sometimes the RosterPage needs to be loaded first
		QTimer::singleShot(300, [=] () {
			emit xmppUriReceived(openUriCache);
			openUriCache = "";
		});
	}
}

void Kaidan::setDisconnReason(Enums::DisconnectionReason reason)
{
	disconnReason = reason;
	emit disconnReasonChanged((quint8) reason);
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

	caches->settings->setValue("auth/resource", jidResource);
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

	this->chatPartner = chatPartner;
	emit chatPartnerChanged(chatPartner);
	caches->msgModel->applyRecipientFilter(chatPartner);
}

quint8 Kaidan::getDisconnReason() const
{
	return (quint8) disconnReason;
}

QString Kaidan::getResourcePath(QString name) const
{
	// We generally prefer to first search for files in application resources
	if (QFile::exists(":/" + name))
		return QString("qrc:/" + name);

	// list of file paths where to search for the resource file
	QStringList pathList;
	// add relative path from binary (only works if installed)
	pathList << QCoreApplication::applicationDirPath() + QString("/../share/") + QString(APPLICATION_NAME);
	// get the standard app data locations for current platform
	pathList << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
#ifdef UBUNTU_TOUCH
	pathList << QString("./share/") + QString(APPLICATION_NAME);
#endif
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
			return QUrl::fromLocalFile(directory.absoluteFilePath(name)).toString();
		}
	}

	// no file found
	qWarning() << "[main] Could NOT find media file:" << name;
	return QString("");
}

void Kaidan::addOpenUri(QByteArray uri)
{
	qDebug() << "[main]" << uri;

	if (!uri.startsWith("xmpp:") || !uri.contains("@"))
		return;

	if (connectionState == ConnectionState::StateConnected) {
		emit xmppUriReceived(QString::fromUtf8(uri));
	} else {
		//: The link is an XMPP-URI (i.e. 'xmpp:kaidan@muc.kaidan.im?join' for joining a chat)
		emit passiveNotificationRequested(tr("The link will be opened after you have connected."));
		openUriCache = QString::fromUtf8(uri);
	}
}

void Kaidan::copyToClipboard(QString text)
{
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(text);
}
