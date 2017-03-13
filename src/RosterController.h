/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
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

#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

// Qt
#include <QObject>
#include <QSqlTableModel>
#include <QQmlListProperty>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "RosterModel.h"

class RosterController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(RosterModel* rosterModel READ getRosterModel NOTIFY rosterModelChanged)

public:
	RosterController(QObject *parent = 0);
	~RosterController();

	void setClient(Swift::Client *client_);
	void requestRosterFromClient();
	RosterModel* getRosterModel();
	void updateLastExchangedOfJid(const QString jid_);
	Q_INVOKABLE void addContact(const QString jid_, const QString name_);
	Q_INVOKABLE void removeContact(const QString);

signals:
	void rosterModelChanged();

private:
	void handleRosterReceived(Swift::ErrorPayload::ref error);
	void handleInitialRosterPopulated();
	void handleJidAdded(const Swift::JID &jid_);
	void handleJidRemoved(const Swift::JID &jid_);
	void handleJidUpdated(const Swift::JID &jid_, const std::string &name_, const std::vector<std::string>&);
	void handleRosterCleared();
	Swift::Client* client;
	Swift::IQRouter *iqRouter;
	Swift::XMPPRoster* xmppRoster;
	RosterModel* rosterModel;
};

#endif // ROSTERCONTROLLER_H
