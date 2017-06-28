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

#ifndef KAIDAN_H
#define KAIDAN_H

// Qt
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
// gloox
#include <gloox/client.h>
#include <gloox/connectionlistener.h>
// Kaidan
#include "Database.h"
#include "RosterManager.h"
#include "MessageSessionHandler.h"
#include "PresenceController.h"
#include "VCardController.h"
#include "ServiceDiscoveryManager.h"

class Kaidan : public QObject, public gloox::ConnectionListener
{
	Q_OBJECT

	Q_PROPERTY(RosterModel* rosterModel READ getRosterModel NOTIFY rosterModelChanged)
	Q_PROPERTY(MessageModel* messageModel READ getMessageModel NOTIFY messageModelChanged)
	Q_PROPERTY(VCardController* vCardController READ getVCardController NOTIFY vCardControllerChanged)
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
	VCardController* getVCardController();

	virtual void onConnect();
	virtual void onDisconnect(gloox::ConnectionError error);
	virtual bool onTLSConnect(const gloox::CertInfo &info);

signals:
	void rosterModelChanged();
	void messageModelChanged();
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
	gloox::Client *client;

	Database *database;
	RosterModel *rosterModel;
	RosterManager *rosterManager;
	MessageModel *messageModel;
	MessageSessionHandler *messageSessionHandler;
	PresenceController *presenceController;
	VCardController *vCardController;
	ServiceDiscoveryManager *serviceDiscoveryManager;
	QSettings *settings;

	bool connected;
	QString jid;
	QString jidResource;
	QString password;
	QString chatPartner;
};

#endif
