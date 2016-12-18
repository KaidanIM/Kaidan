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

#ifndef DATABASE_H
#define DATABASE_H

// Qt
#include <QObject>
#include <QSqlDatabase>

class Database : public QObject
{
	Q_OBJECT

public:
	explicit Database(QObject *parent = 0);
	bool isValid();
	QSqlDatabase* getPointer();

	void dumpDataToStdOut() const;

signals:

public slots:

private:
	bool databaseValid_;
	QSqlDatabase database_;
};

#endif // DATABASE_H
