/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QSqlTableModel>
#include "Enums.h"

using namespace Enums;

class QMimeType;

class MessageModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MessageModel(QSqlDatabase *database, QObject *parent = nullptr);

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

	/**
	 * Applies a filter to the database to only show messages of a certain chat
	 */
	void applyRecipientFilter(QString recipient);

	struct Message {
		QString author;
		QString authorResource;
		QString recipient;
		QString recipientResource;
		QString timestamp;
		QString message;
		QString id;
		bool sentByMe;
		MessageType type;
		QString mediaUrl;
		quint64 mediaSize;
		QString mediaContentType;
		quint64 mediaLastModified;
		QString mediaLocation;
		QByteArray mediaThumb;
		QString mediaHashes;
	};

	static MessageType messageTypeFromMimeType(const QMimeType &);

signals:
	/**
	 * Emitted when the user opens another chat to apply a filter to the db
	 */
	void chatPartnerChanged(QString &jid);
	void addMessageRequested(Message msg);
	void setMessageAsSentRequested(const QString msgId);
	void setMessageAsDeliveredRequested(const QString msgId);
	void updateMessageRequested(const QString id, Message msg);

public slots:
	/**
	 * Set own JID for displaying correct messages
	 */
	void setOwnJid(const QString &jid)
	{
		ownJid = jid;
	}

private slots:
	void addMessage(Message msg);
	void setMessageAsSent(const QString msgId);
	void setMessageAsDelivered(const QString msgId);
	void updateMessage(const QString id, Message msg);

private:
	QSqlDatabase *database;

	QString ownJid;
};

#endif // MESSAGEMODEL_H
