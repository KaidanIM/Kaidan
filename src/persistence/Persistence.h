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

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>

class Database;
class MessageController;
class SessionController;

class Persistence : public QObject
{
	Q_OBJECT
	Q_PROPERTY(MessageController* messageController READ getMessageController NOTIFY messageControllerChanged)

public:
	explicit Persistence(QObject *parent = 0);
	~Persistence();

	bool isValid();
	void markMessageAsReceivedById(QString const &id);

signals:
	void messageControllerChanged();

public slots:
	void addMessage(const QString &id, QString const &jid, QString const &message, unsigned int direction);
	void setCurrentChatPartner(QString const &jid);

private:
	MessageController* getMessageController();

	Database *db_;
	MessageController *messageController_;

	bool persistenceValid_;
};

#endif // PERSISTENCE_H
