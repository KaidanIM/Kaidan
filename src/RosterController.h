/*
 *  Kaidan - Cross platform XMPP client
 *
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
#include <QQmlListProperty>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "RosterItem.h"


class RosterController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<RosterItem> rosterList READ getRosterList NOTIFY rosterListChanged)

public:
	RosterController(QObject *parent = 0);

	void requestRosterFromClient(Swift::Client *client);
	QQmlListProperty<RosterItem> getRosterList();

signals:
	void rosterListChanged();

public slots:

private:
	void handleRosterReceived(Swift::ErrorPayload::ref error);

	Swift::Client* client_;
	QList<RosterItem*> rosterList_;

};

#endif // ROSTERCONTROLLER_H
