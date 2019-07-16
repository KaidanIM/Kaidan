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

// Qt
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <qqml.h>

// QXmpp
#include "qxmpp-exts/QXmppUploadManager.h"
#include <QXmppClient.h>

// Kaidan
#include "AvatarFileStorage.h"
#include "EmojiModel.h"
#include "Enums.h"
#include "Globals.h"
#include "Kaidan.h"
#include "Message.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "QmlUtils.h"
#include "RosterModel.h"
#include "StatusBar.h"
#include "UploadManager.h"
#include "EmojiModel.h"
#include "Utils.h"
#include "QrCodeScannerFilter.h"

#ifdef STATIC_BUILD
#include "static_plugins.h"
#endif

#ifndef QAPPLICATION_CLASS
#define QAPPLICATION_CLASS QApplication
#endif
#include QT_STRINGIFY(QAPPLICATION_CLASS)

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
// SingleApplication (Qt5 replacement for QtSingleApplication)
#include "singleapp/singleapplication.h"
#endif

#ifdef STATIC_BUILD
#define KIRIGAMI_BUILD_TYPE_STATIC
#include "./3rdparty/kirigami/src/kirigamiplugin.h"
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

enum CommandLineParseResult {
	CommandLineOk,
	CommandLineError,
	CommandLineVersionRequested,
	CommandLineHelpRequested
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QString *errorMessage)
{
	// application description
	parser.setApplicationDescription(QString(APPLICATION_DISPLAY_NAME) +
	                                 " - " + QString(APPLICATION_DESCRIPTION));

	// add all possible arguments
	QCommandLineOption helpOption = parser.addHelpOption();
	QCommandLineOption versionOption = parser.addVersionOption();
	parser.addOption({"disable-xml-log", "Disable output of full XMPP XML stream."});
	parser.addOption({{"m", "multiple"}, "Allow multiple instances to be started."});
	parser.addPositionalArgument("xmpp-uri", "An XMPP-URI to open (i.e. join a chat).",
	                             "[xmpp-uri]");

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

Q_DECL_EXPORT int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
#endif
	// initialize random generator
	qsrand(time(nullptr));

	//
	// App
	//

#ifdef UBUNTU_TOUCH
	qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "true");
	qputenv("QT_QUICK_CONTROLS_MOBILE", "true");
#endif

#ifdef APPIMAGE
	qputenv("OPENSSL_CONF", "");
#endif

	// name, display name, description
	QGuiApplication::setApplicationName(APPLICATION_NAME);
	QGuiApplication::setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
	QGuiApplication::setApplicationVersion(VERSION_STRING);
	// attributes
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	// create a qt app
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
	QGuiApplication app(argc, argv);
#else
	SingleApplication app(argc, argv, true);
#endif

	// register qMetaTypes
	qRegisterMetaType<RosterItem>("RosterItem");
	qRegisterMetaType<RosterModel*>("RosterModel*");
	qRegisterMetaType<Message>("Message");
	qRegisterMetaType<MessageModel*>("MessageModel*");
	qRegisterMetaType<AvatarFileStorage*>("AvatarFileStorage*");
	qRegisterMetaType<PresenceCache*>("PresenceCache*");
	qRegisterMetaType<QXmppPresence>("QXmppPresence");
	qRegisterMetaType<ClientWorker::Credentials>("Credentials");
	qRegisterMetaType<Qt::ApplicationState>("Qt::ApplicationState");
	qRegisterMetaType<QXmppClient::State>("QXmppClient::State");
	qRegisterMetaType<MessageType>("MessageType");
	qRegisterMetaType<DisconnectionReason>("DisconnectionReason");
	qRegisterMetaType<TransferJob*>("TransferJob*");
	qRegisterMetaType<QmlUtils*>("QmlUtils*");
	qRegisterMetaType<QVector<Message>>("QVector<Message>");
	qRegisterMetaType<QVector<RosterItem>>("QVector<RosterItem>");
	qRegisterMetaType<QHash<QString,RosterItem>>("QHash<QString,RosterItem>");
	qRegisterMetaType<std::function<void(RosterItem&)>>("std::function<void(RosterItem&)>");
	qRegisterMetaType<std::function<void(Message&)>>("std::function<void(Message&)>");
	qRegisterMetaType<ClientWorker::Credentials>("ClientWorker::Credentials");
	// Enums for c++ member calls using enums
	qRegisterMetaType<Enums::ConnectionState>();
	qRegisterMetaType<Enums::DisconnectionReason>();
	qRegisterMetaType<Enums::MessageType>();
	qRegisterMetaType<Enums::AvailabilityTypes>();

	// Qt-Translator
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(),
	                  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	QCoreApplication::installTranslator(&qtTranslator);

	// Kaidan-Translator
	QTranslator kaidanTranslator;
	// load the systems locale or none from resources
	kaidanTranslator.load(QLocale::system().name(), ":/i18n");
	QCoreApplication::installTranslator(&kaidanTranslator);

	//
	// Command line arguments
	//

	// create parser and add a description
	QCommandLineParser parser;
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

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
	// check if another instance already runs
	if (app.isSecondary() && !parser.isSet("multiple")) {
		qDebug().noquote() << QString("Another instance of %1 is already running.")
		                      .arg(APPLICATION_DISPLAY_NAME)
		                   << "You can enable multiple instances by specifying '--multiple'.";

		// send a possible link to the primary instance
		if (!parser.positionalArguments().isEmpty())
			app.sendMessage(parser.positionalArguments().first().toUtf8());
		return 0;
	}
#endif


	//
	// Kaidan back-end
	//

	Kaidan kaidan(&app, !parser.isSet("disable-xml-log"));

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
	// receive messages from other instances of Kaidan
	Kaidan::connect(&app, &SingleApplication::receivedMessage,
			&kaidan, &Kaidan::receiveMessage);
#endif

	// open the XMPP-URI/link (if given)
	if (!parser.positionalArguments().isEmpty())
		kaidan.addOpenUri(parser.positionalArguments().first());

	//
	// QML-GUI
	//

	if (QIcon::themeName().isEmpty()) {
		QIcon::setThemeName("breeze");
	}

	QQmlApplicationEngine engine;
	// QtQuickControls2 Style
	if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
#ifdef Q_OS_WIN
		const QString defaultStyle = QStringLiteral("Universal");
#else
		const QString defaultStyle = QStringLiteral("Material");
#endif
		qDebug() << "QT_QUICK_CONTROLS_STYLE not set, setting to" << defaultStyle;
		qputenv("QT_QUICK_CONTROLS_STYLE", defaultStyle.toLatin1());
	}
	// QML type bindings
#ifdef STATIC_BUILD
	KirigamiPlugin::getInstance().registerTypes();
#endif
	qmlRegisterType<StatusBar>("StatusBar", 0, 1, "StatusBar");
	qmlRegisterType<EmojiModel>("EmojiModel", 0, 1, "EmojiModel");
	qmlRegisterType<EmojiProxyModel>("EmojiModel", 0, 1, "EmojiProxyModel");
	qmlRegisterType<QrCodeScannerFilter>(APPLICATION_ID, 1, 0, "QrCodeScannerFilter");

	qmlRegisterUncreatableType<QAbstractItemModel>("EmojiModel", 0, 1, "QAbstractItemModel", "Used by proxy models");
	qmlRegisterUncreatableType<Emoji>("EmojiModel", 0, 1, "Emoji", "Used by emoji models");
	qmlRegisterUncreatableType<TransferJob>(APPLICATION_ID, 1, 0, "TransferJob", "TransferJob type usable");

	qmlRegisterUncreatableMetaObject(Enums::staticMetaObject, APPLICATION_ID,
		1, 0, "Enums", "Can't create object; only enums defined!");

	engine.rootContext()->setContextProperty("kaidan", &kaidan);
	engine.load(QUrl("qrc:/qml/main.qml"));
	if(engine.rootObjects().isEmpty())
		return -1;

#ifdef Q_OS_ANDROID
	QtAndroid::hideSplashScreen();
#endif

	// enter qt main loop
	return app.exec();
}
