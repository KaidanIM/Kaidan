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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QSqlTableModel>

class MessageModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MessageModel(QSqlDatabase *database, QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

	void applyRecipientFilter(QString *recipient_, QString *author_);
	void addMessage(const QString *author, const QString *author_resource,
	                const QString *recipient, const QString *recipient_resource,
	                const QString *timestamp, const QString *message,
	                const QString *msgId, bool sentByMe);
	void setMessageAsSent(const QString msgId);
	void setMessageAsDelivered(const QString msgId);

signals:
	void recipientChanged();

private:
	QSqlDatabase* database;
};

#endif // MESSAGEMODEL_H
