/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016 LNJ <git@lnj.li>
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
#include <QStringList>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"
#include "Persistence.h"


class RosterController;
class Persistence;

class Kaidan : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
	Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
	Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)

public:
	Kaidan(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Kaidan();

	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
	Q_INVOKABLE void sendMessage(QString const &toJid, QString const &message);
	Q_INVOKABLE void setCurrentChatPartner(QString const &jid);

	bool connectionState() const;

signals:
	void rosterControllerChanged();
	void persistenceChanged();

	void connectionStateConnected();
	void connectionStateDisconnected();

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	void handleConnected();
	void handleDisconnected();
	void handleMessageReceived(Swift::Message::ref message);
	bool connected;

	RosterController* getRosterController();
	Persistence* getPersistence();

	Swift::Client* client_;
	Swift::ClientXMLTracer* tracer_;
	Swift::SoftwareVersionResponder* softwareVersionResponder_;
	EchoPayloadParserFactory echoPayloadParserFactory_;
	EchoPayloadSerializer echoPayloadSerializer_;
	Swift::NetworkFactories *netFactories_;

	RosterController* rosterController_;
	Persistence* persistence_;
};

#endif // KAIDAN_H
