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

#ifndef ROSTERMODEL_H
#define ROSTERMODEL_H

// Qt
#include <QObject>
#include <QSqlTableModel>
#include <QQmlListProperty>

class RosterModel : public QSqlTableModel
{
	Q_OBJECT
public:
	RosterModel(QObject *parent = 0);

	QHash<int, QByteArray> roleNames() const;
	QVariant data(const QModelIndex &index, int role) const;

	void clearData();
	void insertContact(QString, QString);
	void removeContactByJid(QString);
	void updateContactName(QString, QString);
	QStringList getJidList();
	void removeListOfJids(QStringList*);
	void setLastExchangedOfJid(QString*, QString*);
	int getUnreadMessageCountOfJid(const QString* jid_);
	void setUnreadMessageCountOfJid(const QString* jid_, const int unreadMessageCount);
};

#endif // ROSTERMODEL_H
