/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGEDB_H
#define MESSAGEDB_H

#include <functional>
#include <QObject>

class Message;
class QSqlQuery;
class QSqlRecord;

/**
 * @class The MessageDb is used to query the 'messages' database table. It's used by the
 * MessageModel to load messages and by the MessageHandler to insert messages.
 *
 * All queries must be executed only after the Kaidan SQL connection has been opened in
 * the Database class.
 */
class MessageDb : public QObject
{
	Q_OBJECT

public:
	explicit MessageDb(QObject *parent = nullptr);

	/**
	 * Parses a list of messages from a SELECT query.
	 */
	static void parseMessagesFromQuery(QSqlQuery &query, QVector<Message> &msgs);

	/**
	 * Creates an @c QSqlRecord for updating an old message to a new message.
	 *
	 * @param oldMsg Full message as it is currently saved
	 * @param newMsg Full message as it should be after the update query ran.
	 */
	static QSqlRecord createUpdateRecord(const Message &oldMsg,
	                                     const Message &newMsg);

signals:
	/**
	 * Can be used to triggerd fetchMessages()
	 */
	void fetchMessagesRequested(const QString &user1,
	                            const QString &user2,
	                            int index);

	/**
	 * Emitted, when new messages have been fetched
	 */
	void messagesFetched(const QVector<Message> &messages);

public slots:
	/**
	 * @brief Fetches more entries from the database and emits messagesFetched() with
	 * the results.
	 *
	 * @param user1 Messages are from or to this JID.
	 * @param user2 Messages are from or to this JID.
	 * @param index Number of entries to be skipped, used for paging.
	 */
	void fetchMessages(const QString &user1,
	                   const QString &user2,
	                   int index);

	/**
	 * Adds a message to the database.
	 */
	void addMessage(const Message &msg);

	/**
	 * Deletes a message from the database.
	 */
	void removeMessage(const QString &id);

	/**
	 * Removes all messages from the database.
	 */
	void removeAllMessages();

	/**
	 * Loads a message, runs the update lambda and writes it to the DB again.
	 *
	 * @param updateMsg Function that changes the message
	 */
	void updateMessage(const QString &id,
			   const std::function<void (Message &)> &updateMsg);

	/**
	 * Updates message by @c UPDATE record: This means it doesn't load the message
	 * from the database and writes it again, but executes an UPDATE query.
	 *
	 * @param updateRecord
	 */
	void updateMessageRecord(const QString &id,
	                         const QSqlRecord &updateRecord);

	/**
	 * Marks a message as sent using an UPDATE query.
	 */
	void setMessageAsSent(const QString &msgId);

	/**
	 * Marks a message as delivered using an UPDATE query.
	 */
	void setMessageAsDelivered(const QString &msgId);
};

#endif // MESSAGEDB_H
