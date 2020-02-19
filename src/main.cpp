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

// Qt
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QStandardPaths>
#include <qqml.h>

// QXmpp
#include "qxmpp-exts/QXmppUploadManager.h"
#include <QXmppClient.h>

// Kaidan
#include "AvatarFileStorage.h"
#include "EmojiModel.h"
#include "Enums.h"
#include "Kaidan.h"
#include "Message.h"
#include "MessageModel.h"
#include "PresenceCache.h"
#include "QmlUtils.h"
#include "RosterModel.h"
#include "RosterFilterProxyModel.h"
#include "StatusBar.h"
#include "UploadManager.h"
#include "EmojiModel.h"
#include "Utils.h"
#include "QrCodeScannerFilter.h"
#include "VCardModel.h"
#include "CameraModel.h"
#include "AudioDeviceModel.h"
#include "MediaSettingModel.h"
#include "MediaUtils.h"
#include "MediaRecorder.h"
#include "CredentialsValidator.h"

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

#ifdef APPIMAGE
	QFileInfo executable(QCoreApplication::applicationFilePath());

	if (executable.isSymLink()) {
		executable.setFile(executable.symLinkTarget());
	}

	QString gstreamerPluginsPath;

	// Try to use deployed plugins if any...
#if defined(TARGET_GSTREAMER_PLUGINS)
	gstreamerPluginsPath = QString::fromLocal8Bit(TARGET_GSTREAMER_PLUGINS);

	if (!gstreamerPluginsPath.isEmpty()) {
		gstreamerPluginsPath = QDir::cleanPath(QString::fromLatin1("%1/../..%2")
							.arg(executable.absolutePath(), gstreamerPluginsPath));
	}
#else
	qFatal("Please provide the unified directory containing the gstreamer plugins and gst-plugin-scanner.");
#endif

#if defined(QT_DEBUG)
	qputenv("GST_DEBUG", "ERROR:5,WARNING:5,INFO:5,DEBUG:5,LOG:5");
#endif
	qputenv("GST_PLUGIN_PATH_1_0", QByteArray());
	qputenv("GST_PLUGIN_SYSTEM_PATH_1_0", gstreamerPluginsPath.toLocal8Bit());
	qputenv("GST_PLUGIN_SCANNER_1_0", QString::fromLatin1("%1/gst-plugin-scanner").arg(gstreamerPluginsPath).toLocal8Bit());
#endif // APPIMAGE

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
	qRegisterMetaType<TransferJob*>("TransferJob*");
	qRegisterMetaType<QmlUtils*>("QmlUtils*");
	qRegisterMetaType<QVector<Message>>("QVector<Message>");
	qRegisterMetaType<QVector<RosterItem>>("QVector<RosterItem>");
	qRegisterMetaType<QHash<QString,RosterItem>>("QHash<QString,RosterItem>");
	qRegisterMetaType<std::function<void(RosterItem&)>>("std::function<void(RosterItem&)>");
	qRegisterMetaType<std::function<void(Message&)>>("std::function<void(Message&)>");
	qRegisterMetaType<ClientWorker::Credentials>("ClientWorker::Credentials");
	qRegisterMetaType<QXmppVCardIq>("QXmppVCardIq");
	qRegisterMetaType<QMimeType>();
	qRegisterMetaType<CameraInfo>();
	qRegisterMetaType<AudioDeviceInfo>();
	qRegisterMetaType<MediaSettings>();
	qRegisterMetaType<ImageEncoderSettings>();
	qRegisterMetaType<AudioEncoderSettings>();
	qRegisterMetaType<VideoEncoderSettings>();
	qRegisterMetaType<CredentialsValidator*>();
	qRegisterMetaType<QXmppStanza::Error>("QXmppStanza::Error");

	// Enums for c++ member calls using enums
	qRegisterMetaType<Enums::ConnectionState>();
	qRegisterMetaType<ClientWorker::ConnectionError>();
	qRegisterMetaType<Enums::MessageType>();
	qRegisterMetaType<Enums::AvailabilityTypes>();
	qRegisterMetaType<CommonEncoderSettings::EncodingQuality>();
	qRegisterMetaType<CommonEncoderSettings::EncodingMode>();
	qRegisterMetaType<AudioDeviceModel::Mode>();
	qRegisterMetaType<MediaRecorder::Type>();
	qRegisterMetaType<MediaRecorder::AvailabilityStatus>();
	qRegisterMetaType<MediaRecorder::State>();
	qRegisterMetaType<MediaRecorder::Status>();
	qRegisterMetaType<MediaRecorder::Error>();

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
	auto *kaidan = new Kaidan(&app, !parser.isSet("disable-xml-log"));

#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
	// receive messages from other instances of Kaidan
	Kaidan::connect(&app, &SingleApplication::receivedMessage,
	                kaidan, &Kaidan::receiveMessage);
#endif

	// open the XMPP-URI/link (if given)
	if (!parser.positionalArguments().isEmpty())
		kaidan->addOpenUri(parser.positionalArguments().first());

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
	qmlRegisterType<VCardModel>(APPLICATION_ID, 1, 0, "VCardModel");
	qmlRegisterType<RosterFilterProxyModel>(APPLICATION_ID, 1, 0, "RosterFilterProxyModel");
	qmlRegisterType<CameraModel>(APPLICATION_ID, 1, 0, "CameraModel");
	qmlRegisterType<AudioDeviceModel>(APPLICATION_ID, 1, 0, "AudioDeviceModel");
	qmlRegisterType<MediaSettingsContainerModel>(APPLICATION_ID, 1, 0, "MediaSettingsContainerModel");
	qmlRegisterType<MediaSettingsResolutionModel>(APPLICATION_ID, 1, 0, "MediaSettingsResolutionModel");
	qmlRegisterType<MediaSettingsQualityModel>(APPLICATION_ID, 1, 0, "MediaSettingsQualityModel");
	qmlRegisterType<MediaSettingsImageCodecModel>(APPLICATION_ID, 1, 0, "MediaSettingsImageCodecModel");
	qmlRegisterType<MediaSettingsAudioCodecModel>(APPLICATION_ID, 1, 0, "MediaSettingsAudioCodecModel");
	qmlRegisterType<MediaSettingsAudioSampleRateModel>(APPLICATION_ID, 1, 0, "MediaSettingsAudioSampleRateModel");
	qmlRegisterType<MediaSettingsVideoCodecModel>(APPLICATION_ID, 1, 0, "MediaSettingsVideoCodecModel");
	qmlRegisterType<MediaSettingsVideoFrameRateModel>(APPLICATION_ID, 1, 0, "MediaSettingsVideoFrameRateModel");
	qmlRegisterType<MediaRecorder>(APPLICATION_ID, 1, 0, "MediaRecorder");
	qmlRegisterType<CredentialsValidator>(APPLICATION_ID, 1, 0, "CredentialsValidator");

	qmlRegisterUncreatableType<QAbstractItemModel>("EmojiModel", 0, 1, "QAbstractItemModel", "Used by proxy models");
	qmlRegisterUncreatableType<Emoji>("EmojiModel", 0, 1, "Emoji", "Used by emoji models");
	qmlRegisterUncreatableType<TransferJob>(APPLICATION_ID, 1, 0, "TransferJob", "TransferJob type usable");
	qmlRegisterUncreatableType<QMimeType>(APPLICATION_ID, 1, 0, "QMimeType", "QMimeType type usable");
	qmlRegisterUncreatableType<CameraInfo>(APPLICATION_ID, 1, 0, "CameraInfo", "CameraInfo type usable");
	qmlRegisterUncreatableType<AudioDeviceInfo>(APPLICATION_ID, 1, 0, "AudioDeviceInfo", "AudioDeviceInfo type usable");
	qmlRegisterUncreatableType<MediaSettings>(APPLICATION_ID, 1, 0, "MediaSettings", "MediaSettings type usable");
	qmlRegisterUncreatableType<CommonEncoderSettings>(APPLICATION_ID, 1, 0, "CommonEncoderSettings", "CommonEncoderSettings type usable");
	qmlRegisterUncreatableType<ImageEncoderSettings>(APPLICATION_ID, 1, 0, "ImageEncoderSettings", "ImageEncoderSettings type usable");
	qmlRegisterUncreatableType<AudioEncoderSettings>(APPLICATION_ID, 1, 0, "AudioEncoderSettings", "AudioEncoderSettings type usable");
	qmlRegisterUncreatableType<VideoEncoderSettings>(APPLICATION_ID, 1, 0, "VideoEncoderSettings", "VideoEncoderSettings type usable");
	qmlRegisterUncreatableType<ClientWorker>(APPLICATION_ID, 1, 0, "ClientWorker", "Cannot create object; only enums defined!");


	qmlRegisterUncreatableMetaObject(Enums::staticMetaObject, APPLICATION_ID,
		1, 0, "Enums", "Can't create object; only enums defined!");

	qmlRegisterSingletonType<MediaUtils>("MediaUtils", 0, 1, "MediaUtilsInstance", [](QQmlEngine *, QJSEngine *) {
		QObject *instance = new MediaUtils(qApp);
		return instance;
	});
	qmlRegisterSingletonType<QmlUtils>(APPLICATION_ID, 1, 0, "Utils", [](QQmlEngine *, QJSEngine *) {
		return static_cast<QObject*>(QmlUtils::instance());
	});
	qmlRegisterSingletonType<Kaidan>(APPLICATION_ID, 1, 0, "Kaidan", [](QQmlEngine *, QJSEngine *) {
		return static_cast<QObject*>(Kaidan::instance());
	});

	engine.load(QUrl("qrc:/qml/main.qml"));
	if (engine.rootObjects().isEmpty())
		return -1;

#ifdef Q_OS_ANDROID
	QtAndroid::hideSplashScreen();
#endif

	// enter qt main loop
	return app.exec();
}
