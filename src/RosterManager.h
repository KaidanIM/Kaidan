/*
 *  Kaidan - A user-friendly XMPP client for every device!
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

#ifndef ROSTERMANAGER_H
#define ROSTERMANAGER_H

// gloox
#include <gloox/client.h>
#include <gloox/rostermanager.h>
// Kaidan
#include "RosterModel.h"
#include "RosterUpdater.h"

class RosterManager
{
public:
	RosterManager(RosterModel *rosterModel, gloox::Client *client);
	~RosterManager();

	void addContact(const QString jid_, const QString name_);
	void removeContact(const QString);

private:
	RosterModel *rosterModel;
	RosterUpdater *rosterUpdater;
	gloox::RosterManager *rosterManager;
};

#endif // ROSTERMANAGER_H
