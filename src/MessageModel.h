/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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

class MessageModel : public QSqlTableModel
{
	Q_OBJECT

public:
	MessageModel(QSqlDatabase *database, QObject *parent = 0);

	QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

	void applyRecipientFilter(QString *recipient_, QString *author_);
	void addMessage(const QString *author, const QString *recipient,
					const QString *timestamp, const QString *message,
					const QString *msgId, bool sentByMe,
					const QString *author_resource = new QString(),
					const QString *recipient_resource = new QString());
	void setMessageAsSent(const QString msgId);
	void setMessageAsDelivered(const QString msgId);

signals:
	void recipientChanged();

private:
	QSqlDatabase* database;
};

#endif // MESSAGEMODEL_H
