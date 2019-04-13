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

#include "Kaidan.h"

// Qt
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
// QXmpp
#include <QXmppClient.h>
#include "qxmpp-exts/QXmppColorGenerator.h"
// Kaidan
#include "AvatarFileStorage.h"
#include "Database.h"
#include "RosterModel.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "Utils.h"

Kaidan::Kaidan(QGuiApplication *app, bool enableLogging, QObject *parent)
        : QObject(parent), utils(new Utils(this)), database(new Database())
{
	// Database setup
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

	creds.jid = caches->settings->value(KAIDAN_SETTINGS_AUTH_JID).toString();
	creds.jidResource = caches->settings->value(KAIDAN_SETTINGS_AUTH_RESOURCE)
	                    .toString();
	creds.password = QString(QByteArray::fromBase64(caches->settings->value(
	                 KAIDAN_SETTINGS_AUTH_PASSWD).toString().toUtf8()));
	// use Kaidan as resource, if no set
	if (creds.jidResource.isEmpty())
		setJidResource(APPLICATION_DISPLAY_NAME);
	creds.isFirstTry = false;

	//
	// Start ClientWorker on new thread
	//

	cltThrd = new ClientThread();
	client = new ClientWorker(caches, this, enableLogging, app);
	client->setCredentials(creds);
	connect(client, &ClientWorker::disconnReasonChanged, this, &Kaidan::setDisconnReason);

	client->moveToThread(cltThrd);
	connect(cltThrd, &QThread::started, client, &ClientWorker::main);
	cltThrd->start();
}

Kaidan::~Kaidan()
{
	delete caches;
	delete database;
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

void Kaidan::setDisconnReason(DisconnectionReason reason)
{
	disconnReason = reason;
	emit disconnReasonChanged();
}

void Kaidan::setJid(const QString &jid)
{
	creds.jid = jid;
	// credentials were modified -> first try
	creds.isFirstTry = true;
}

void Kaidan::setJidResource(const QString &jidResource)
{
	// JID resource won't influence the authentication, so we don't need
	// to set the first try flag and can save it.
	creds.jidResource = jidResource;

	caches->settings->setValue("auth/resource", jidResource);
}

void Kaidan::setPassword(const QString &password)
{
	creds.password = password;
	// credentials were modified -> first try
	creds.isFirstTry = true;
}

void Kaidan::setChatPartner(const QString &chatPartner)
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

void Kaidan::addOpenUri(const QByteArray &uri)
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
