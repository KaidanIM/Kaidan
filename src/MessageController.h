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

#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

// Qt
#include <QObject>
// Swiften
#include <Swiften/Client/Client.h>
#include <Swiften/Elements/Message.h>
// Kaidan
#include "MessageModel.h"
#include "RosterController.h"

class MessageController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(MessageModel *messageModel READ getMessageModel NOTIFY messageModelChanged)

public:
	MessageController(QString *ownJid_, QObject *parent = 0);
	~MessageController();

	void setClient(Swift::Client *client_);
	MessageModel* getMessageModel();

	void setChatPartner(QString *recipient);
	void sendMessage(QString *recipient_, QString *message_);

signals:
	void messageModelChanged();
	void recipientChanged();

private:
	void handleMessageReceived(Swift::Message::ref message);

	Swift::Client *client;
	MessageModel *messageModel;
	QString *ownJid;
};

#endif // MESSAGECONTROLLER_H
