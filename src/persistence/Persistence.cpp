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

// Qt
#include <QDebug>
// Kaidan
#include "Persistence.h"
#include "Database.h"
#include "MessageController.h"

Persistence::Persistence(QObject *parent) : QObject(parent), persistenceValid_(true)
{
	db_ = new Database(this);
	if (!db_->isValid())
	{
		persistenceValid_ = false;
	}
	else
	{
		qDebug() << "\n\n\n\n\n\n YEEEAAAAHHH \n\n\n\n\n\n";
		messageController_ = new MessageController(db_, this);
	}
}

Persistence::~Persistence()
{
	// db_ has this as parent and gets free'd implicit from this;
}

void Persistence::addMessage(QString const &id, QString const &jid, QString const &message, unsigned int direction)
{
	messageController_->addMessage(id, jid, message, direction);

	emit messageControllerChanged();
}

void Persistence::markMessageAsReceivedById(QString const &id)
{
	messageController_->markMessageReceived(id);
}

void Persistence::setCurrentChatPartner(QString const &jid)
{
	qDebug() << "persistence::setCurrentChatPartner(" << jid << ")";
	messageController_->setFilterOnJid(jid);
}

bool Persistence::isValid()
{
	return persistenceValid_;
}

MessageController* Persistence::getMessageController()
{
	return messageController_;
}
