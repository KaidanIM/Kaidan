/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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
#include "Globals.h"
#include "Utils.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

static const int DATABASE_LATEST_VERSION = 10;

Database::Database(QObject *parent)
    : QObject(parent)
{
}

Database::~Database()
{
	m_database.close();
}

void Database::openDatabase()
{
	m_database = QSqlDatabase::addDatabase("QSQLITE", DB_CONNECTION);
	if (!m_database.isValid())
		qFatal("Cannot add database: %s", qPrintable(m_database.lastError().text()));

	const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!writeDir.mkpath(".")) {
		qFatal("Failed to create writable directory at %s", qPrintable(writeDir.absolutePath()));
	}

	// Ensure that we have a writable location on all devices.
	const QString fileName = writeDir.absoluteFilePath(DB_FILENAME);
	// open() will create the SQLite database if it doesn't exist.
	m_database.setDatabaseName(fileName);
	if (!m_database.open()) {
		qFatal("Cannot open database: %s", qPrintable(m_database.lastError().text()));
	}

	loadDatabaseInfo();

	if (needToConvert())
		convertDatabase();
}

void Database::transaction()
{
	if (!m_transactions) {
		// currently no transactions running
		if (!m_database.transaction()) {
			qWarning() << "Could not begin transaction on database:"
			           << m_database.lastError().text();
		}
	}
	// increase counter
	m_transactions++;
}

void Database::commit()
{
	// reduce counter
	m_transactions--;
	Q_ASSERT(m_transactions >= 0);

	if (!m_transactions) {
		// no transaction requested anymore
		if (!m_database.commit()) {
			qWarning() << "Could not commit transaction on database:"
			           << m_database.lastError().text();
		}
	}
}

void Database::loadDatabaseInfo()
{
	QStringList tables = m_database.tables();
	if (!tables.contains(DB_TABLE_INFO)) {
		if (tables.contains(DB_TABLE_MESSAGES) &&
			tables.contains(DB_TABLE_ROSTER))
			// old Kaidan v0.1/v0.2 table
			m_version = 1;
		else
			m_version = 0;
		// we've got all we want; do not query for a db version
		return;
	}

	QSqlQuery query(m_database);
	Utils::execQuery(query, "SELECT version FROM dbinfo");

	QSqlRecord record = query.record();
	int versionCol = record.indexOf("version");

	while (query.next()) {
		m_version = query.value(versionCol).toInt();
	}
}

bool Database::needToConvert()
{
	return m_version < DATABASE_LATEST_VERSION;
}

void Database::convertDatabase()
{
	qDebug() << "[database] Converting database to latest version from version" << m_version;
	transaction();
	while (m_version < DATABASE_LATEST_VERSION) {
		switch (m_version) {
		case 0:
			createNewDatabase(); m_version = DATABASE_LATEST_VERSION; break;
		case 1:
			convertDatabaseToV2(); m_version = 2; break;
		case 2:
			convertDatabaseToV3(); m_version = 3; break;
		case 3:
			convertDatabaseToV4(); m_version = 4; break;
		case 4:
			convertDatabaseToV5(); m_version = 5; break;
		case 5:
			convertDatabaseToV6(); m_version = 6; break;
		case 6:
			convertDatabaseToV7(); m_version = 7; break;
		case 7:
			convertDatabaseToV8(); m_version = 8; break;
		case 8:
			convertDatabaseToV9(); m_version = 9; break;
		case 9:
			convertDatabaseToV10(); m_version = 10; break;
		default:
			break;
		}

		// TODO: the next time we change the messages table, we need to do:
		//        * rename author to sender, edited to isEdited
		//        * delete author_resource, recipient_resource
		//        * remove 'NOT NULL' from id
	}

	QSqlRecord updateRecord;
	updateRecord.append(Utils::createSqlField("version", DATABASE_LATEST_VERSION));

	QSqlQuery query(m_database);
	Utils::execQuery(
	        query,
	        m_database.driver()->sqlStatement(
	                QSqlDriver::UpdateStatement,
	                DB_TABLE_INFO,
	                updateRecord,
	                false
	        )
	);

	commit();
	m_version = DATABASE_LATEST_VERSION;
}

void Database::createNewDatabase()
{
	QSqlQuery query(m_database);

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
	    "'spoilerHint' TEXT," //spoiler hint if isSpoiler
	    "'isSpoiler' BOOL," // message is spoiler
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
	QSqlQuery query(m_database);
	Utils::execQuery(
	        query,
	        "CREATE TABLE IF NOT EXISTS 'dbinfo' (version INTEGER NOT NULL)"
	);


	QSqlRecord insertRecord;
	insertRecord.append(Utils::createSqlField("version", DATABASE_LATEST_VERSION));

	Utils::execQuery(
	        query,
	        m_database.driver()->sqlStatement(
	                QSqlDriver::InsertStatement,
	                DB_TABLE_INFO,
	                insertRecord,
	                false
	        )
	);
}

void Database::convertDatabaseToV2()
{
	// create a new dbinfo table
	createDbInfoTable();
}

void Database::convertDatabaseToV3()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Roster ADD avatarHash TEXT");
}

void Database::convertDatabaseToV4()
{
	QSqlQuery query(m_database);
	// SQLite doesn't support the ALTER TABLE drop columns feature, so we have to use a workaround.
	// we copy all rows into a back-up table (but without `avatarHash`), and then delete the old table
	// and copy everything to the normal table again
	Utils::execQuery(query, "CREATE TEMPORARY TABLE roster_backup(jid,name,lastExchanged,"
	                        "unreadMessages,lastMessage,lastOnline,activity,status,mood);");
	Utils::execQuery(query, "INSERT INTO roster_backup SELECT jid,name,lastExchanged,unreadMessages,"
	                        "lastMessage,lastOnline,activity,status,mood FROM Roster;");
	Utils::execQuery(query, "DROP TABLE Roster;");
	Utils::execQuery(query, "CREATE TABLE Roster('jid' TEXT NOT NULL,'name' TEXT NOT NULL,"
	                        "'lastExchanged' TEXT NOT NULL,'unreadMessages' INTEGER,'lastMessage' TEXT,"
	                        "'lastOnline' TEXT,'activity' TEXT,'status' TEXT,'mood' TEXT);");
	Utils::execQuery(query, "INSERT INTO Roster SELECT jid,name,lastExchanged,unreadMessages,"
	                        "lastMessage,lastOnline,activity,status,mood FROM Roster_backup;");
	Utils::execQuery(query, "DROP TABLE Roster_backup;");
}

void Database::convertDatabaseToV5()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'type' INTEGER");
	Utils::execQuery(query, "UPDATE Messages SET type = 0 WHERE type IS NULL");
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'mediaUrl' TEXT");
}

void Database::convertDatabaseToV6()
{
	QSqlQuery query(m_database);
	for (const QString &column : {"'mediaSize' INTEGER",
		     "'mediaContentType' TEXT",
		     "'mediaLastModified' INTEGER",
		     "'mediaLocation' TEXT"}) {
		Utils::execQuery(query, QString("ALTER TABLE 'Messages' ADD ").append(column));
	}
}

void Database::convertDatabaseToV7()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'mediaThumb' BLOB");
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'mediaHashes' TEXT");
}

void Database::convertDatabaseToV8()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "CREATE TEMPORARY TABLE roster_backup(jid, name, lastExchanged, "
	                        "unreadMessages, lastMessage);");
	Utils::execQuery(query, "INSERT INTO roster_backup SELECT jid, name, lastExchanged, unreadMessages, "
	                        "lastMessage FROM Roster;");
	Utils::execQuery(query, "DROP TABLE Roster;");
	Utils::execQuery(query, "CREATE TABLE IF NOT EXISTS Roster ('jid' TEXT NOT NULL,'name' TEXT,"
	                        "'lastExchanged' TEXT NOT NULL, 'unreadMessages' INTEGER,"
	                        "'lastMessage' TEXT);");
	Utils::execQuery(query, "INSERT INTO Roster SELECT jid, name, lastExchanged, unreadMessages, "
	                        "lastMessage FROM Roster_backup;");
	Utils::execQuery(query, "DROP TABLE roster_backup;");
}

void Database::convertDatabaseToV9()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'edited' BOOL");
}

void Database::convertDatabaseToV10()
{
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'isSpoiler' BOOL");
	Utils::execQuery(query, "ALTER TABLE 'Messages' ADD 'spoilerHint' TEXT");
}
