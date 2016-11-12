#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Kaidan.h"
#include "RosterContoller.h"
#include "RosterItem.h"

int main(int argc, char *argv[])
{
    qmlRegisterType<RosterController>( "harbour.kaidan", 1, 0, "RosterController");
    qmlRegisterType<RosterItem>( "harbour.kaidan", 1, 0, "RosterItem");

    QGuiApplication app(argc, argv);

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
