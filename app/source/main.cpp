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


#if 0
    CarManager manager;

        view->engine()->addImportPath("/usr/share/harbour-carbudget/qml");
        view->rootContext()->setContextProperty("manager", &manager);
        view->setSource(SailfishApp::pathTo("qml/Application.qml"));
        view->showFullScreen();
#endif

#if 0
        QQuickView view;
        view.setResizeMode(QQuickView::SizeRootObjectToView);
        QQmlContext *ctxt = view.rootContext();
        ctxt->setContextProperty("myModel", QVariant::fromValue(dataList));
#endif

    return app.exec();
}
