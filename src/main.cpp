// Qt
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>
// Swiften
#include "Swiften/EventLoop/Qt/QtEventLoop.h"
// Kaidan
#include "Kaidan.h"
#include "RosterController.h"
#include "RosterItem.h"

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

int main(int argc, char *argv[])
{
	qmlRegisterType<RosterController>(APPLICATION_ID, 1, 0, "RosterController");
	qmlRegisterType<RosterItem>(APPLICATION_ID, 1, 0, "RosterItem");

	// create a qt app
	QGuiApplication app(argc, argv);
	QGuiApplication::setApplicationName(APPLICATION_NAME);
	QGuiApplication::setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
	QGuiApplication::setApplicationVersion(VERSION_STRING);

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
			qDebug() << "Successfully parsed command line input.";
			break;
	}

	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);

	Kaidan kaidan(&networkFactories);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("kaidan", &kaidan);

	engine.load(QUrl("qrc:/qml/main.qml"));
	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	window->show();
	return app.exec();
}
