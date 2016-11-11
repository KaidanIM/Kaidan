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

    QQuickView view;
    QQmlContext *ctxt = view.rootContext();
    ctxt->setContextProperty("kaidan", &kaidan);

    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

    return app.exec();
}
