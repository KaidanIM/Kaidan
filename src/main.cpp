// Qt
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

int main(int argc, char *argv[])
{
	qmlRegisterType<RosterController>(APPLICATION_ID, 1, 0, "RosterController");
	qmlRegisterType<RosterItem>(APPLICATION_ID, 1, 0, "RosterItem");

	QGuiApplication app(argc, argv);
	QGuiApplication::setApplicationName(APPLICATION_NAME);
	QGuiApplication::setApplicationDisplayName(APPLICATION_DISPLAY_NAME);
	QGuiApplication::setApplicationVersion(VERSION_STRING);

	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);

	Kaidan kaidan(&networkFactories);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("kaidan", &kaidan);

	engine.load(QUrl("qrc:/main.qml"));
	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	window->show();
	return app.exec();
}
