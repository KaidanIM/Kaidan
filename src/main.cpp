/*
 *  Kaidan - A user-friendly XMPP client for every device!
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
#include <Swiften/Network/BoostNetworkFactories.h>
// Kaidan
#include "Kaidan.h"
#include "VCard.h"

enum CommandLineParseResult {
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
	if (!parser.parse(QGuiApplication::arguments())) {
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

	// attributes
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	// Kaidan QML types
	qmlRegisterType<VCard>(APPLICATION_ID, 1, 0, "VCard");

	// Qt-Translator
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
	                  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	// Kaidan-Translator
	QTranslator kaidanTranslator;
	// load the systems locale or none
	kaidanTranslator.load(QString(APPLICATION_NAME) + "_" + QLocale::system().name(),
	                      ":/i18n"); // load the qm files via. rcc (bundled in binary)
	app.installTranslator(&kaidanTranslator);


	//
	// Kaidan back-end
	//

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
	switch (parseCommandLine(parser, &commandLineErrorMessage)) {
	case CommandLineError:
		qWarning() << commandLineErrorMessage;
		return 1;
	case CommandLineVersionRequested:
		parser.showVersion();
		return 0;
	case CommandLineHelpRequested:
		parser.showHelp();
		return 0;
	case CommandLineOk:
		break;
	}


	//
	// QML-GUI
	//
#ifndef SAILFISH_OS
	// QtQuickControls2 Style
	qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
#endif

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
