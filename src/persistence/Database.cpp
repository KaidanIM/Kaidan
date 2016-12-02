#include "Database.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

Database::Database(QObject *parent) : QObject(parent), databaseValid_(true)
{
	database_ = QSqlDatabase::addDatabase("QSQLITE");
	if (! database_.isValid())
	{
		qDebug() << "Database error!";
		databaseValid_ = false;
	}
	else
	{
		// TODO place in filesystem when tables get stable
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
