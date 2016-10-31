#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "Swiften/EventLoop/Qt/QtEventLoop.h"

#include "EchoBot.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // FIXME rewrite binds to qt specific versions
    // FIXME test connects from QtEventLoop (Swiften) to QGuiApplication

    QtEventLoop eventLoop;
    BoostNetworkFactories networkFactories(&eventLoop);

    EchoBot bot(&networkFactories);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
