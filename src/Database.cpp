/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include "Kaidan.h"

#define DATABASE_CONVERT_TO_VERSION(n) \
	if (m_version < n) { \
		convertDatabaseToV##n(); \
	}

// Both need to be updated on version bump:
#define DATABASE_LATEST_VERSION 13
#define DATABASE_CONVERT_TO_LATEST_VERSION() DATABASE_CONVERT_TO_VERSION(13)

#define SQL_BOOL "BOOL"
#define SQL_INTEGER "INTEGER"
#define SQL_INTEGER_NOT_NULL "INTEGER NOT NULL"
#define SQL_TEXT "TEXT"
#define SQL_TEXT_NOT_NULL "TEXT NOT NULL"
#define SQL_BLOB "BLOB"

#define SQL_CREATE_TABLE(tableName, contents) \
	"CREATE TABLE '" tableName "' (" contents ")"

#define SQL_LAST_ATTRIBUTE(name, dataType) \
	"'" QT_STRINGIFY(name) "' " dataType

#define SQL_ATTRIBUTE(name, dataType) \
	SQL_LAST_ATTRIBUTE(name, dataType) ","

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
	Utils::execQuery(query, "SELECT version FROM " DB_TABLE_INFO);

	QSqlRecord record = query.record();
	int versionCol = record.indexOf("version");

	while (query.next()) {
		m_version = query.value(versionCol).toInt();
	}
}

void Database::saveDatabaseInfo()
{
	if (m_version < 2 || m_version > DATABASE_LATEST_VERSION)
		qFatal("[database] Fatal error: Attempted to save invalid db version number.");

	QSqlRecord updateRecord;
	updateRecord.append(Utils::createSqlField("version", m_version));

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
}

bool Database::needToConvert()
{
	return m_version < DATABASE_LATEST_VERSION;
}

void Database::convertDatabase()
{
	qDebug() << "[database] Converting database to latest version from version" << m_version;
	transaction();

	if (m_version == 0)
		createNewDatabase();
	else
		DATABASE_CONVERT_TO_LATEST_VERSION();

	saveDatabaseInfo();
	commit();
}

void Database::createNewDatabase()
{
	createDbInfoTable();
	createRosterTable();
	createMessagesTable();

	m_version = DATABASE_LATEST_VERSION;
}

void Database::createDbInfoTable()
{
	QSqlQuery query(m_database);
	Utils::execQuery(
		query,
		SQL_CREATE_TABLE(
			DB_TABLE_INFO,
			SQL_LAST_ATTRIBUTE(version, SQL_INTEGER_NOT_NULL)
		)
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

void Database::createRosterTable()
{
	// TODO: remove lastExchanged and lastMessage

	QSqlQuery query(m_database);
	Utils::execQuery(
		query,
		SQL_CREATE_TABLE(
			DB_TABLE_ROSTER,
			SQL_ATTRIBUTE(jid, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(name, SQL_TEXT)
			SQL_ATTRIBUTE(lastExchanged, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(unreadMessages, SQL_INTEGER)
			SQL_LAST_ATTRIBUTE(lastMessage, SQL_TEXT)
		)
	);
}

void Database::createMessagesTable()
{
	// TODO: the next time we change the messages table, we need to do:
	//  * rename author to sender, edited to isEdited
	//  * delete author_resource, recipient_resource
	//  * remove 'NOT NULL' from id
	//  * remove columns isSent, isDelivered

	QSqlQuery query(m_database);
	Utils::execQuery(
		query,
		SQL_CREATE_TABLE(
			DB_TABLE_MESSAGES,
			SQL_ATTRIBUTE(author, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(author_resource, SQL_TEXT)
			SQL_ATTRIBUTE(recipient, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(recipient_resource, SQL_TEXT)
			SQL_ATTRIBUTE(timestamp, SQL_TEXT)
			SQL_ATTRIBUTE(message, SQL_TEXT)
			SQL_ATTRIBUTE(id, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(isSent, SQL_BOOL)
			SQL_ATTRIBUTE(isDelivered, SQL_BOOL)
			SQL_ATTRIBUTE(deliveryState, SQL_INTEGER)
			SQL_ATTRIBUTE(type, SQL_INTEGER)
			SQL_ATTRIBUTE(mediaUrl, SQL_TEXT)
			SQL_ATTRIBUTE(mediaSize, SQL_INTEGER)
			SQL_ATTRIBUTE(mediaContentType, SQL_TEXT)
			SQL_ATTRIBUTE(mediaLastModified, SQL_INTEGER)
			SQL_ATTRIBUTE(mediaLocation, SQL_TEXT)
			SQL_ATTRIBUTE(mediaThumb, SQL_BLOB)
			SQL_ATTRIBUTE(mediaHashes, SQL_TEXT)
			SQL_ATTRIBUTE(edited, SQL_BOOL)
			SQL_ATTRIBUTE(spoilerHint, SQL_TEXT)
			SQL_ATTRIBUTE(isSpoiler, SQL_BOOL)
			SQL_ATTRIBUTE(errorText, SQL_TEXT)
			SQL_ATTRIBUTE(replaceId, SQL_TEXT)
			SQL_ATTRIBUTE(originId, SQL_TEXT)
			SQL_ATTRIBUTE(stanzaId, SQL_TEXT)
			"FOREIGN KEY(author) REFERENCES " DB_TABLE_ROSTER " (jid),"
			"FOREIGN KEY(recipient) REFERENCES " DB_TABLE_ROSTER " (jid)"
		)
	);
}

void Database::convertDatabaseToV2()
{
	// create a new dbinfo table
	createDbInfoTable();
	m_version = 2;
}

void Database::convertDatabaseToV3()
{
	DATABASE_CONVERT_TO_VERSION(2);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Roster ADD avatarHash " SQL_TEXT);
	m_version = 3;
}

void Database::convertDatabaseToV4()
{
	DATABASE_CONVERT_TO_VERSION(3);
	QSqlQuery query(m_database);
	// SQLite doesn't support the ALTER TABLE drop columns feature, so we have to use a workaround.
	// we copy all rows into a back-up table (but without `avatarHash`), and then delete the old table
	// and copy everything to the normal table again
	Utils::execQuery(query, "CREATE TEMPORARY TABLE roster_backup(jid, name, lastExchanged,"
							"unreadMessages, lastMessage, lastOnline, activity, status, mood)");
	Utils::execQuery(query, "INSERT INTO roster_backup SELECT jid, name, lastExchanged, unreadMessages,"
							"lastMessage, lastOnline, activity, status, mood FROM " DB_TABLE_ROSTER);
	Utils::execQuery(query, "DROP TABLE Roster");
	Utils::execQuery(query, "CREATE TABLE Roster (jid " SQL_TEXT_NOT_NULL ", name " SQL_TEXT_NOT_NULL ","
							"lastExchanged " SQL_TEXT_NOT_NULL ", unreadMessages " SQL_INTEGER ", lastMessage  " SQL_TEXT ","
							"lastOnline " SQL_TEXT ", activity " SQL_TEXT ", status " SQL_TEXT ", mood " SQL_TEXT ")");
	Utils::execQuery(query, "INSERT INTO Roster SELECT jid, name, lastExchanged, unreadMessages,"
							"lastMessage, lastOnline, activity, status, mood FROM Roster_backup");
	Utils::execQuery(query, "DROP TABLE Roster_backup");
	m_version = 4;
}

void Database::convertDatabaseToV5()
{
	DATABASE_CONVERT_TO_VERSION(4);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD type " SQL_INTEGER);
	Utils::execQuery(query, "UPDATE Messages SET type = 0 WHERE type IS NULL");
	Utils::execQuery(query, "ALTER TABLE Messages ADD mediaUrl " SQL_TEXT);
	m_version = 5;
}

void Database::convertDatabaseToV6()
{
	DATABASE_CONVERT_TO_VERSION(5);
	QSqlQuery query(m_database);
	for (const QString &column : {	"mediaSize " SQL_INTEGER,
									"mediaContentType " SQL_TEXT,
									"mediaLastModified " SQL_INTEGER,
									"mediaLocation " SQL_TEXT }) {
		Utils::execQuery(query, QString("ALTER TABLE Messages ADD ").append(column));
	}
	m_version = 6;
}

void Database::convertDatabaseToV7()
{
	DATABASE_CONVERT_TO_VERSION(6);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD mediaThumb " SQL_BLOB);
	Utils::execQuery(query, "ALTER TABLE Messages ADD mediaHashes " SQL_TEXT);
	m_version = 7;
}

void Database::convertDatabaseToV8()
{
	DATABASE_CONVERT_TO_VERSION(7);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "CREATE TEMPORARY TABLE roster_backup(jid, name, lastExchanged, unreadMessages, lastMessage)");
	Utils::execQuery(query, "INSERT INTO roster_backup SELECT jid, name, lastExchanged, unreadMessages, lastMessage FROM Roster");
	Utils::execQuery(query, "DROP TABLE Roster");
	Utils::execQuery(
		query,
		SQL_CREATE_TABLE(
			"Roster",
			SQL_ATTRIBUTE(jid, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(name, SQL_TEXT)
			SQL_ATTRIBUTE(lastExchanged, SQL_TEXT_NOT_NULL)
			SQL_ATTRIBUTE(unreadMessages, SQL_INTEGER)
			SQL_LAST_ATTRIBUTE(lastMessage, SQL_TEXT_NOT_NULL)
		)
	);

	Utils::execQuery(query, "INSERT INTO Roster SELECT jid, name, lastExchanged, unreadMessages, lastMessage FROM Roster_backup");
	Utils::execQuery(query, "DROP TABLE roster_backup");
	m_version = 8;
}

void Database::convertDatabaseToV9()
{
	DATABASE_CONVERT_TO_VERSION(8);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD edited " SQL_BOOL);
	m_version = 9;
}

void Database::convertDatabaseToV10()
{
	DATABASE_CONVERT_TO_VERSION(9);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD isSpoiler " SQL_BOOL);
	Utils::execQuery(query, "ALTER TABLE Messages ADD spoilerHint " SQL_TEXT);
	m_version = 10;
}

void Database::convertDatabaseToV11()
{
	DATABASE_CONVERT_TO_VERSION(10);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD deliveryState " SQL_INTEGER);
	Utils::execQuery(query, "UPDATE Messages SET deliveryState = 2 WHERE isDelivered = 1");
	Utils::execQuery(query, "UPDATE Messages SET deliveryState = 1 WHERE deliveryState IS NULL");
	Utils::execQuery(query, "ALTER TABLE Messages ADD errorText " SQL_TEXT);
	m_version = 11;
}

void Database::convertDatabaseToV12()
{
	DATABASE_CONVERT_TO_VERSION(11);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD replaceId " SQL_TEXT);
	m_version = 12;
}

void Database::convertDatabaseToV13()
{
	DATABASE_CONVERT_TO_VERSION(12);
	QSqlQuery query(m_database);
	Utils::execQuery(query, "ALTER TABLE Messages ADD stanzaId " SQL_TEXT);
	Utils::execQuery(query, "ALTER TABLE Messages ADD originId " SQL_TEXT);
	m_version = 13;
}
