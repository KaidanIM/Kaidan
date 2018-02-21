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

#ifndef KAIDAN_H
#define KAIDAN_H

// Qt
#include <QObject>
#include <QSettings>
#include <QString>
#include <QTimer>
// gloox
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
// Kaidan
#include "branding.h"
#include "Database.h"
#include "RosterManager.h"
#include "MessageSessionHandler.h"
#include "PresenceHandler.h"
#include "ServiceDiscoveryManager.h"
#include "VCardManager.h"
#include "AvatarFileStorage.h"
#include "XmlLogHandler.h"

class LinkPreviewFetcher;

class Kaidan : public QObject, public gloox::ConnectionListener
{
	Q_OBJECT

	Q_PROPERTY(RosterModel* rosterModel READ getRosterModel NOTIFY rosterModelChanged)
	Q_PROPERTY(MessageModel* messageModel READ getMessageModel NOTIFY messageModelChanged)
	Q_PROPERTY(AvatarFileStorage* avatarStorage READ getAvatarStorage NOTIFY avatarStorageChanged)
	Q_PROPERTY(bool connectionState READ getConnectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString jidResource READ getJidResource WRITE setJidResource NOTIFY jidResourceChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged)
	Q_PROPERTY(QString chatPartner READ getChatPartner WRITE setChatPartner NOTIFY chatPartnerChanged)

public:
	Kaidan(QObject *parent = 0);
	~Kaidan();

	Q_INVOKABLE void mainConnect();
	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE bool newLoginNeeded();
	Q_INVOKABLE void sendMessage(QString jid, QString message);
	Q_INVOKABLE void addContact(QString jid, QString nick);
	Q_INVOKABLE void removeContact(QString jid);
	Q_INVOKABLE QString getResourcePath(QString);
	Q_INVOKABLE QString getVersionString();
	Q_INVOKABLE QString removeNewLinesFromString(QString input);

	bool getConnectionState() const;
	QString getJid();
	void setJid(QString);
	QString getJidResource();
	void setJidResource(QString);
	QString getPassword();
	void setPassword(QString);
	QString getChatPartner();
	void setChatPartner(QString);
	RosterModel* getRosterModel();
	MessageModel* getMessageModel();
	AvatarFileStorage* getAvatarStorage();

	virtual void onConnect();
	virtual void onDisconnect(gloox::ConnectionError error);
	virtual bool onTLSConnect(const gloox::CertInfo &info);

signals:
	void rosterModelChanged();
	void messageModelChanged();
	void avatarStorageChanged();
	void vCardControllerChanged();
	void connectionStateConnected();
	void connectionStateDisconnected();
	void jidChanged();
	void jidResourceChanged();
	void passwordChanged();
	void chatPartnerChanged();

private slots:
	void updateClient();

private:
	void clientCleanUp();

	gloox::Client *client;
	Database *database;
	RosterModel *rosterModel;
	RosterManager *rosterManager;
	MessageModel *messageModel;
	MessageSessionHandler *messageSessionHandler;
	PresenceHandler *presenceHandler;
	ServiceDiscoveryManager *serviceDiscoveryManager;
	VCardManager *vCardManager;
	AvatarFileStorage *avatarStorage;
	XmlLogHandler *xmlLogHandler;
	QSettings *settings;

	bool connected;
	bool isClientSetUp;
	QString jid;
	QString jidResource;
	QString password;
	QString chatPartner;
	QTimer *packageFetchTimer;

	LinkPreviewFetcher *linkPreviewer;
};

#endif
