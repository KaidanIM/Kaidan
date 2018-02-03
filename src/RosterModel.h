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

#ifndef ROSTERMODEL_H
#define ROSTERMODEL_H

// Qt
#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QQmlListProperty>

class RosterModel : public QSqlTableModel
{
	Q_OBJECT
public:
	RosterModel(QSqlDatabase *database, QObject *parent = 0);

	QHash<int, QByteArray> roleNames() const;
	QVariant data(const QModelIndex &index, int role) const;

	void clearData();
	void insertContact(QString, QString);
	void removeContactByJid(QString);
	void updateContactName(QString, QString);
	QStringList getJidList();
	void removeListOfJids(QStringList*);
	void setLastExchangedOfJid(const QString *jid, QString *date);
	int getUnreadMessageCountOfJid(const QString* jid_);
	void setUnreadMessageCountOfJid(const QString* jid_, const int unreadMessageCount);
	void setLastMessageForJid(const QString *jid, QString *message);

private:
	QSqlDatabase *database;
};

#endif // ROSTERMODEL_H
