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

#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QSqlTableModel>

class Database;

class MessageController : public QSqlTableModel
{
	Q_OBJECT

public:
	explicit MessageController(QObject *parent = 0);
	MessageController(Database *db, QObject *parent = 0);

	Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
	virtual QHash<int, QByteArray> roleNames() const;

	void setFilterOnJid(QString const &jidFiler);
    void addMessage(const QString &id, QString const &jid, QString const &message, unsigned int direction);
    void markMessageReceived(QString const &id);

signals:
	void signalMessageReceived(unsigned int id, QString jid, QString message);

public slots:

private:
	void generateRoleNames();
	virtual void setTable ( const QString &table_name );
    int getRowNumberForId(QString const &id);

    void printSqlError();

	QHash<int, QByteArray> roles_;
	Database *database_;
};

#endif // MESSAGECONTROLLER_H
