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
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "RosterController.h"
#include "MessageController.h"
#include "PresenceController.h"
#include "VCardController.h"
#include "ServiceDiscoveryManager.h"

class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
	Q_PROPERTY(MessageController* messageController READ getMessageController NOTIFY messageControllerChanged)
	Q_PROPERTY(VCardController* vCardController READ getVCardController NOTIFY vCardControllerChanged)
	Q_PROPERTY(bool connectionState READ getConnectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString jidResource READ getJidResource WRITE setJidResource NOTIFY jidResourceChanged)
	Q_PROPERTY(QString password READ getPassword WRITE setPassword NOTIFY passwordChanged)

public:
	Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Kaidan();

	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect();

	bool getConnectionState() const;
	Q_INVOKABLE bool newLoginNeeded();

	QString getJid();
	QString getJidResource();
	QString getPassword();
	void setJid(QString);
	void setJidResource(QString);
	void setPassword(QString);

	RosterController* getRosterController();
	MessageController* getMessageController();
	VCardController* getVCardController();

	Q_INVOKABLE QString getResourcePath(QString);
	Q_INVOKABLE QString getVersionString();

signals:
	void rosterControllerChanged();
	void messageControllerChanged();
	void vCardControllerChanged();
	void connectionStateConnected();
	void connectionStateDisconnected();
	void jidChanged();
	void jidResourceChanged();
	void passwordChanged();

private:
	void handleConnected();
	void handleDisconnected();
	void updateFullJid();

	bool connected;

	Swift::Client* client;
	Swift::ClientXMLTracer* tracer;
	Swift::SoftwareVersionResponder* softwareVersionResponder;
	Swift::NetworkFactories *netFactories;

	RosterController* rosterController;
	MessageController* messageController;
	PresenceController* presenceController;
	VCardController* vCardController;
	ServiceDiscoveryManager* serviceDiscoveryManager;

	QSettings* settings;

	QString jid;
	QString jidResource;
	QString fullJid;
	QString password;
};

#endif
