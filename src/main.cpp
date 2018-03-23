/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

// Qt
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#if HAVE_QWIDGETS
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QLocale>
#include <qqml.h>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QLibraryInfo>
// Kaidan
#include "Kaidan.h"
#include "RosterModel.h"
#include "MessageModel.h"
#include "AvatarFileStorage.h"
#include "Globals.h"
#include "Enums.h"
#include "StatusBar.h"

#ifdef QMAKE_BUILD
#include "./3rdparty/kirigami/src/kirigamiplugin.h"
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

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
#if HAVE_QWIDGETS
	QApplication app(argc, argv);
#else
	QGuiApplication app(argc, argv);
#endif

	// name, display name, description
	QGuiApplication::setApplicationName(APPLICATION_NAME);
	QGuiApplication::setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
	QGuiApplication::setApplicationVersion(VERSION_STRING);

	// attributes
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	// register qMetaTypes
	qRegisterMetaType<RosterModel*>("RosterModel*");
	qRegisterMetaType<MessageModel*>("MessageModel*");
	qRegisterMetaType<AvatarFileStorage*>("AvatarFileStorage*");
	qRegisterMetaType<ContactMap>("ContactMap");
	qRegisterMetaType<Qt::ApplicationState>("Qt::ApplicationState");
	qmlRegisterUncreatableMetaObject(Enums::staticMetaObject, APPLICATION_ID,
	                                 1, 0, "Kaidan", "Access to enums & flags only");

	// Qt-Translator
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
	                  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);

	// Kaidan-Translator
	QTranslator kaidanTranslator;
	// load the systems locale or none from resources
	kaidanTranslator.load(QLocale::system().name(), ":/i18n");
	app.installTranslator(&kaidanTranslator);


	//
	// Kaidan back-end
	//

	Kaidan kaidan(&app);

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

	QQmlApplicationEngine engine;

#ifndef SAILFISH_OS
	// QtQuickControls2 Style
	if (qgetenv("QT_QUICK_CONTROLS_STYLE").isEmpty()) {
		qDebug() << "QT_QUICK_CONTROLS_STYLE not set, setting to Material";
		qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
	}
#endif

#ifdef UBUNTU_TOUCH
       qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "true");
#endif

	// QML type bindings
#ifdef QMAKE_BUILD
	KirigamiPlugin::getInstance().registerTypes();
#endif
	qmlRegisterType<StatusBar>("StatusBar", 0, 1, "StatusBar");
	qmlRegisterUncreatableMetaObject(Enums::staticMetaObject, APPLICATION_ID,
		1, 0, "Enums", "Can't create object; only enums defined!");

	engine.rootContext()->setContextProperty("kaidan", &kaidan);
	engine.load(QUrl("qrc:/qml/main.qml"));
	if(engine.rootObjects().isEmpty())
		return -1;

#ifdef Q_OS_ANDROID
	//TODO: use QQmlApplicationEngine::objectCreated to ensure QML was actually loaded?
	QtAndroid::hideSplashScreen();
#endif

	// enter qt main loop
	return app.exec();
}
