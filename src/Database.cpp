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

#include "Database.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

static const int DATABASE_LATEST_VERSION = 9;
static const char *DATABASE_TABLE_INFO = "dbinfo";
static const char *DATABASE_TABLE_MESSAGES = "Messages";
static const char *DATABASE_TABLE_ROSTER = "Roster";

Database::Database(QObject *parent) : QObject(parent)
{
	version = -1;

	database = QSqlDatabase::addDatabase("QSQLITE", "kaidan_default_db");
	if (!database.isValid()) {
		qFatal("Cannot add database: %s", qPrintable(database.lastError().text()));
	}
}

Database::~Database()
{
	database.close();
}

QSqlDatabase* Database::getDatabase()
{
	return &database;
}

void Database::openDatabase()
{
	const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!writeDir.mkpath(".")) {
		qFatal("Failed to create writable directory at %s", qPrintable(writeDir.absolutePath()));
	}

	// Ensure that we have a writable location on all devices.
	const QString fileName = writeDir.absoluteFilePath("messages.sqlite3");
	// open() will create the SQLite database if it doesn't exist.
	database.setDatabaseName(fileName);
	if (!database.open()) {
		qFatal("Cannot open database: %s", qPrintable(database.lastError().text()));
		QFile::remove(fileName);
	}

	loadDatabaseInfo();
}

void Database::loadDatabaseInfo()
{
	QStringList tables = database.tables();
	if (!tables.contains(DATABASE_TABLE_INFO)) {
		if (tables.contains(DATABASE_TABLE_MESSAGES) &&
			tables.contains(DATABASE_TABLE_ROSTER)) {
			// old Kaidan v0.1/v0.2 table
			version = 1;
		} else {
			version = 0;
		}
		// we've got all we want; do not query for a db version
		return;
	}

	QSqlQuery query(database);
	query.prepare("SELECT version FROM dbinfo");
	if (!query.exec()) {
		qWarning("Cannot query database info: %s", qPrintable(database.lastError().text()));
	}

	QSqlRecord record = query.record();
	int versionCol = record.indexOf("version");

	while (query.next()) {
		version = query.value(versionCol).toInt();
	}
}

bool Database::needToConvert()
{
	if (version < DATABASE_LATEST_VERSION) {
		return true;
	}
	return false;
}

void Database::convertDatabase()
{
	qDebug() << "[database] Converting database to latest version from version" << version;
	while (version < DATABASE_LATEST_VERSION) {
		switch (version) {
		case 0:
			createNewDatabase(); version = DATABASE_LATEST_VERSION; break;
		case 1:
			convertDatabaseToV2(); version = 2; break;
		case 2:
			convertDatabaseToV3(); version = 3; break;
		case 3:
			convertDatabaseToV4(); version = 4; break;
		case 4:
			convertDatabaseToV5(); version = 5; break;
		case 5:
			convertDatabaseToV6(); version = 6; break;
		case 6:
			convertDatabaseToV7(); version = 7; break;
		case 7:
			convertDatabaseToV8(); version = 8; break;
		case 8:
			convertDatabaseToV9(); version = 9; break;
		default:
			break;
		}
	}

	QSqlQuery query(database);
	query.prepare(QString("UPDATE dbinfo SET version = %1").arg(DATABASE_LATEST_VERSION));
	if (!query.exec()) {
		qDebug("Failed to query database: %s", qPrintable(query.lastError().text()));
	}

	database.commit();
	version = DATABASE_LATEST_VERSION;
}

void Database::createNewDatabase()
{
	QSqlQuery query(database);

	//
	// DB info
	//

	createDbInfoTable();

	//
	// Roster
	//

	if (!query.exec("CREATE TABLE IF NOT EXISTS 'Roster' ("
	                "'jid' TEXT NOT NULL,"
	                "'name' TEXT,"
	                "'lastExchanged' TEXT NOT NULL,"
	                "'unreadMessages' INTEGER,"
	                "'lastMessage' TEXT"
	                ")"))
	{
		qFatal("Error creating roster table: Failed to query database: %s", qPrintable(query.lastError().text()));
	}

	//
	// Messages
	//

	if (!query.exec("CREATE TABLE IF NOT EXISTS 'Messages' ("
	    "'author' TEXT NOT NULL,"
	    "'author_resource' TEXT,"
	    "'recipient' TEXT NOT NULL,"
	    "'recipient_resource' TEXT,"
	    "'timestamp' TEXT NOT NULL,"
	    "'message' TEXT NOT NULL,"
	    "'id' TEXT NOT NULL,"
	    "'isSent' BOOL,"      // is sent to server
	    "'isDelivered' BOOL," // message has arrived at other client
	    "'type' INTEGER,"     // type of message (text/image/video/...)
	    "'mediaUrl' TEXT,"
	    "'mediaSize' INTEGER,"
	    "'mediaContentType' TEXT,"
	    "'mediaLastModified' INTEGER,"
	    "'mediaLocation' TEXT,"
	    "'mediaThumb' BLOB,"
	    "'mediaHashes' TEXT,"
	    "'edited' BOOL," // whether the message has been edited
	    "FOREIGN KEY('author') REFERENCES Roster ('jid'),"
	    "FOREIGN KEY('recipient') REFERENCES Roster ('jid')"
	    ")"
	   ))
	{
		qFatal("Error creating messages table: Failed to query database: %s", qPrintable(query.lastError().text()));
	}
}

void Database::createDbInfoTable()
{
	QSqlQuery query(database);
	query.prepare("CREATE TABLE IF NOT EXISTS 'dbinfo' (version INTEGER NOT NULL)");
	execQuery(query);

	query.prepare(QString("INSERT INTO 'dbinfo' (version) VALUES (%1)")
		.arg(DATABASE_LATEST_VERSION));
	execQuery(query);
}

void Database::convertDatabaseToV2()
{
	// create a new dbinfo table
	createDbInfoTable();
}

void Database::convertDatabaseToV3()
{
	QSqlQuery query(database);
	query.prepare("ALTER TABLE Roster ADD avatarHash TEXT");
	execQuery(query);
}

void Database::convertDatabaseToV4()
{
	QSqlQuery query(database);
	// SQLite doesn't support the ALTER TABLE drop columns feature, so we have to use a workaround.
	// we copy all rows into a back-up table (but without `avatarHash`), and then delete the old table
	// and copy everything to the normal table again
	query.prepare("CREATE TEMPORARY TABLE roster_backup(jid,name,lastExchanged,"
		"unreadMessages,lastMessage,lastOnline,activity,status,mood);");
	execQuery(query);

	query.prepare("INSERT INTO roster_backup SELECT jid,name,lastExchanged,unreadMessages,"
		"lastMessage,lastOnline,activity,status,mood FROM Roster;");
	execQuery(query);

	query.prepare("DROP TABLE Roster;");
	execQuery(query);

	query.prepare("CREATE TABLE Roster('jid' TEXT NOT NULL,'name' TEXT NOT NULL,"
		"'lastExchanged' TEXT NOT NULL,'unreadMessages' INTEGER,'lastMessage' TEXT,"
		"'lastOnline' TEXT,'activity' TEXT,'status' TEXT,'mood' TEXT);");
	execQuery(query);

	query.prepare("INSERT INTO Roster SELECT jid,name,lastExchanged,unreadMessages,"
		"lastMessage,lastOnline,activity,status,mood FROM Roster_backup;");
	execQuery(query);

	query.prepare("DROP TABLE Roster_backup;");
	execQuery(query);
}

void Database::convertDatabaseToV5()
{
	QSqlQuery query(database);
	query.prepare("ALTER TABLE 'Messages' ADD 'type' INTEGER");
	execQuery(query);

	query.prepare("UPDATE Messages SET type = 0 WHERE type IS NULL");
	execQuery(query);

	query.prepare("ALTER TABLE 'Messages' ADD 'mediaUrl' TEXT");
	execQuery(query);
}

void Database::convertDatabaseToV6()
{
	QSqlQuery query(database);
	for (QString column : {"'mediaSize' INTEGER", "'mediaContentType' TEXT",
	                       "'mediaLastModified' INTEGER", "'mediaLocation' TEXT"}) {
		query.prepare(QString("ALTER TABLE 'Messages' ADD ").append(column));
		execQuery(query);
	}
}

void Database::convertDatabaseToV7()
{
	QSqlQuery query(database);
	query.prepare(QString("ALTER TABLE 'Messages' ADD 'mediaThumb' BLOB"));
	execQuery(query);
	query.prepare(QString("ALTER TABLE 'Messages' ADD 'mediaHashes' TEXT"));
	execQuery(query);
}

void Database::convertDatabaseToV8()
{
	QSqlQuery query(database);

	query.prepare("CREATE TEMPORARY TABLE roster_backup(jid, name, lastExchanged, "
	              "unreadMessages, lastMessage);");
	execQuery(query);

	query.prepare("INSERT INTO roster_backup SELECT jid, name, lastExchanged, unreadMessages, "
	              "lastMessage FROM Roster;");
	execQuery(query);

	query.prepare("DROP TABLE Roster;");
	execQuery(query);

	query.prepare("CREATE TABLE IF NOT EXISTS Roster ('jid' TEXT NOT NULL,'name' TEXT,"
	              "'lastExchanged' TEXT NOT NULL, 'unreadMessages' INTEGER,"
	              "'lastMessage' TEXT);");
	execQuery(query);

	query.prepare("INSERT INTO Roster SELECT jid, name, lastExchanged, unreadMessages, "
	              "lastMessage FROM Roster_backup;");
	execQuery(query);

	query.prepare("DROP TABLE roster_backup;");
	execQuery(query);
}

void Database::convertDatabaseToV9()
{
	QSqlQuery query(database);

	query.prepare("ALTER TABLE 'Messages' ADD 'edited' BOOL");
	execQuery(query);
}

void Database::execQuery(QSqlQuery &query)
{
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
}
