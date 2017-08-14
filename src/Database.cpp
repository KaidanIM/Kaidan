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

static const unsigned int DATABASE_LATEST_VERSION = 4;
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
	qDebug() << "[Database] Converting database to latest version from verion" << version;
	switch (version) {
	case 0:
		createNewDatabase();
		break;
	case 1:
		convertDatabaseToV2(); version = 2;
	case 2:
		convertDatabaseToV3(); version = 3;
	case 3:
		convertDatabaseToV4(); version = 4;
		// only break on last convertion step, to not enter default (!)
		break;
	default:
		createNewDatabase();
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
			"'name' TEXT NOT NULL,"
			"'lastExchanged' TEXT NOT NULL,"
			"'unreadMessages' INTEGER,"
			"'lastMessage' TEXT,"
			"'lastOnline' TEXT,"     // < UNUSED v
			"'activity' TEXT,"
			"'status' TEXT,"
			"'mood' TEXT,"           // < UNUSED ^
	                ")"))
	{
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}

	//
	// Messages
	//

	if (!query.exec(
	            "CREATE TABLE IF NOT EXISTS 'Messages' ("
	            "'author' TEXT NOT NULL,"
	            "'author_resource' TEXT,"
	            "'recipient' TEXT NOT NULL,"
	            "'recipient_resource' TEXT,"
	            "'timestamp' TEXT NOT NULL,"
	            "'message' TEXT NOT NULL,"
	            "'id' TEXT NOT NULL,"
	            "'isSent' BOOL,"      // is sent to server
	            "'isDelivered' BOOL," // message has arrived at other client
	            "FOREIGN KEY('author') REFERENCES Roster ('jid'),"
	            "FOREIGN KEY('recipient') REFERENCES Roster ('jid')"
	            ")"))
	{
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
}

void Database::createDbInfoTable()
{
	QSqlQuery query(database);
	query.prepare("CREATE TABLE IF NOT EXISTS 'dbinfo' (version INTEGER NOT NULL)");

	if (!query.exec()) {
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	
	query.prepare(QString("INSERT INTO 'dbinfo' (version) VALUES (%1)")
		.arg(DATABASE_LATEST_VERSION));
	if (!query.exec()) {
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
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
	if (!query.exec())
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
}

void Database::convertDatabaseToV4()
{
	QSqlQuery query(database);
	// SQLite doesn't support the ALTER TABLE drop columns feature, so we have to use a workaround.
	// we copy all rows into a back-up table (but without `avatarHash`), and then delete the old table
	// and copy everything to the normal table again
	query.prepare("CREATE TEMPORARY TABLE roster_backup(jid,name,lastExchanged,"
		"unreadMessages,lastMessage,lastOnline,activity,status,mood);");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	query.prepare("INSERT INTO roster_backup SELECT jid,name,lastExchanged,unreadMessages,"
		"lastMessage,lastOnline,activity,status,mood FROM Roster;");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	query.prepare("DROP TABLE Roster;");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	query.prepare("CREATE TABLE Roster('jid' TEXT NOT NULL,'name' TEXT NOT NULL,"
		"'lastExchanged' TEXT NOT NULL,'unreadMessages' INTEGER,'lastMessage' TEXT,"
		"'lastOnline' TEXT,'activity' TEXT,'status' TEXT,'mood' TEXT);");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	query.prepare("INSERT INTO Roster SELECT jid,name,lastExchanged,unreadMessages,"
		"lastMessage,lastOnline,activity,status,mood FROM Roster_backup;");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
	query.prepare("DROP TABLE Roster_backup;");
	if (!query.exec()) {
		qDebug() << query.executedQuery();
		qFatal("Failed to query database: %s", qPrintable(query.lastError().text()));
	}
}
