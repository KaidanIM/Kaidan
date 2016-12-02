#ifdef SFOS
#include <sailfishapp.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>

#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "Kaidan.h"
#include "RosterContoller.h"
#include "RosterItem.h"
#include "Persistence.h"
#include "MessageController.h"

int main(int argc, char *argv[])
{
	qmlRegisterType<RosterController>( "harbour.kaidan", 1, 0, "RosterController");
	qmlRegisterType<RosterItem>( "harbour.kaidan", 1, 0, "RosterItem");

	qRegisterMetaType<MessageController*>("MessageController*");
	qRegisterMetaType<Persistence*>("Persistence*");

#ifdef SFOS
	QGuiApplication *app = SailfishApp::application(argc, argv);
	QQuickView *view = SailfishApp::createView();
#else
	QGuiApplication app(argc, argv);
#endif

	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);
	Kaidan kaidan(&networkFactories);

#ifdef SFOS
	view->rootContext()->setContextProperty("kaidan", &kaidan);

	view->setSource(QUrl::fromLocalFile("/usr/share/harbour-kaidan/qml/main.qml"));
	view->showFullScreen();

	return app->exec();
#else
	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty("kaidan", &kaidan);
	engine.load(QUrl("qrc:/main.qml"));

	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	window->show();
	return app.exec();
#endif
}
