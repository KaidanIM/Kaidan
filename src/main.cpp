/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
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

// Qt
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTranslator>
#include <QtQml>
// Swiften
#include <Swiften/EventLoop/Qt/QtEventLoop.h>
// Kaidan
#include "Kaidan.h"

enum CommandLineParseResult
{
	CommandLineOk,
	CommandLineError,
	CommandLineVersionRequested,
	CommandLineHelpRequested
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QString *errorMessage)
{
	// add all possible arguments
	QCommandLineOption helpOption = parser.addHelpOption();
	QCommandLineOption versionOption = parser.addVersionOption();

	// parse arguments
	if (!parser.parse(QGuiApplication::arguments()))
	{
		*errorMessage = parser.errorText();
		return CommandLineError;
	}

	// check for special cases
	if (parser.isSet(versionOption))
		return CommandLineVersionRequested;

	if (parser.isSet(helpOption))
		return CommandLineHelpRequested;

	// if nothing special happened, return OK
	return CommandLineOk;
}

void connectToDatabase()
{
	QSqlDatabase database = QSqlDatabase::database();
	if (!database.isValid()) {
		database = QSqlDatabase::addDatabase("QSQLITE");
		if (!database.isValid())
			qFatal("Cannot add database: %s", qPrintable(database.lastError().text()));
	}

	const QDir writeDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	if (!writeDir.mkpath("."))
		qFatal("Failed to create writable directory at %s", qPrintable(writeDir.absolutePath()));

	// Ensure that we have a writable location on all devices.
	const QString fileName = writeDir.absolutePath() + "/messages.sqlite3";
	// When using the SQLite driver, open() will create the SQLite database if it doesn't exist.
	database.setDatabaseName(fileName);
	if (!database.open()) {
		qFatal("Cannot open database: %s", qPrintable(database.lastError().text()));
		QFile::remove(fileName);
	}
}

int main(int argc, char *argv[])
{
	//
	// App
	//

	// create a qt app
	QGuiApplication app(argc, argv);

	// name, display name, description
	QGuiApplication::setApplicationName(APPLICATION_NAME);
	QGuiApplication::setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
	QGuiApplication::setApplicationVersion(VERSION_STRING);
	QGuiApplication::setOrganizationName(ORGANIZAITON_NAME);
	QGuiApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);

	// attributes
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	// Qt-Translator
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	// Kaidan-Translator
	QTranslator kaidanTranslator;
	kaidanTranslator.load(QLocale::system().name()); // loads the systems locale or none
	app.installTranslator(&kaidanTranslator);


	//
	// Kaidan back-end
	//

	// open the message/roster db
	connectToDatabase();

	Swift::QtEventLoop eventLoop;
	Swift::BoostNetworkFactories networkFactories(&eventLoop);

	Kaidan kaidan(&networkFactories);


	//
	// Command line arguments
	//

	// create parser and add a description
	QCommandLineParser parser;
	parser.setApplicationDescription(QString(APPLICATION_DISPLAY_NAME) +
		" - " + QString(APPLICATION_DESCRIPTION));

	// parse the arguments
	QString commandLineErrorMessage;
	switch (parseCommandLine(parser, &commandLineErrorMessage))
	{
		case CommandLineError:
			qDebug() << commandLineErrorMessage;
			return 1;
		case CommandLineVersionRequested:
			parser.showVersion();
			return 0;
		case CommandLineHelpRequested:
			parser.showHelp();
			return 0;
		case CommandLineOk:
			qDebug() << QCoreApplication::translate("main",
				"Successfully parsed command line input.");
			break;
	}


	//
	// QML-GUI
	//

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("kaidan", &kaidan);

	engine.load(QUrl("qrc:/qml/main.qml"));
	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	window->show();

	// execute the app
	int retvar = app.exec();

	// be sure that kaidan has disconnected properly
	kaidan.mainDisconnect();

	return retvar;
}
