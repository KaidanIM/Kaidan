/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
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

#ifndef PRESENCECONTROLLER_H
#define PRESENCECONTROLLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class PresenceController : public QObject
{
	Q_OBJECT

public:
	PresenceController(QObject *parent = 0);
	~PresenceController();

	void setClient(Swift::Client* client_);

signals:

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	Swift::Client* client;
};

#endif // PRESENCECONTROLLER_H
