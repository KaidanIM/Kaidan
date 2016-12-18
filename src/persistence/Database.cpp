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

#include "Database.h"

// Qt
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>


Database::Database(QObject *parent) : QObject(parent), databaseValid_(true)
{
	database_ = QSqlDatabase::addDatabase("QSQLITE");
	if (!database_.isValid())
	{
		qDebug() << "Database error!";
		databaseValid_ = false;
	}
	else
	{
		// TODO: place in filesystem when tables get stable
		database_.setDatabaseName(":memory:");
		if (database_.open() == false)
		{
			qDebug() << "Error open database!";
			databaseValid_ = false;
		}
		else
		{
			QSqlQuery query;
			// direction: (1)ncomming / (0)utgoing
			QString sqlCreateCommand = "create table messages (id INTEGER PRIMARY KEY AUTOINCREMENT, jid TEXT, message TEXT, direction INTEGER, received TEXT)";
			if (query.exec(sqlCreateCommand) == false)
			{
				qDebug() << "Error creating table";
				databaseValid_ = false;
			}
		}
	}
}

QSqlDatabase* Database::getPointer()
{
	return &database_;
}

bool Database::isValid()
{
	return databaseValid_;
}

void Database::dumpDataToStdOut() const
{
	QSqlQuery query("select * from messages", database_);
	QSqlRecord rec = query.record();

	const unsigned int idCol = rec.indexOf("id");
	const unsigned int jidCol = rec.indexOf("jid");
	const unsigned int messageCol = rec.indexOf("message");
	const unsigned int directionCol = rec.indexOf("direction");
	const unsigned int timeStampCol = rec.indexOf("received");

	qDebug() << "id:\t\tjid:\t\tmessage:\t\tdirection\t\treceived:";
	qDebug() << "---------------------------------------------------------------------------------------";
	while (query.next())
	{
		qDebug() << query.value(idCol).toInt() << "\t"
			<< query.value(jidCol).toString() << "\t"
			<< query.value(messageCol).toString() << "\t"
			<< query.value(directionCol).toString() << "\t"
			<< query.value(timeStampCol).toString() << "\t";
	}
}
