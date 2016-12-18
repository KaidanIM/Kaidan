/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016 LNJ <git@lnj.li>
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

// Qt
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QtQml>
// Swiften
#include <Swiften/EventLoop/Qt/QtEventLoop.h>
// Sailfish-specific
#ifdef SFOS
#include <sailfishapp.h>
#endif
// Kaidan
#include "Kaidan.h"
#include "RosterController.h"
#include "RosterItem.h"
#include "Persistence.h"
#include "MessageController.h"


int main(int argc, char *argv[])
{
	// register types for qml
	// TODO: Do we really want to call it harbour.kadan?
	qmlRegisterType<RosterController>("harbour.kaidan", 1, 0, "RosterController");
	qmlRegisterType<RosterItem>("harbour.kaidan", 1, 0, "RosterItem");

	qRegisterMetaType<MessageController*>("MessageController*");
	qRegisterMetaType<Persistence*>("Persistence*");

	//
	// The Back-End
	//

	QtEventLoop eventLoop;
	BoostNetworkFactories networkFactories(&eventLoop);

	Kaidan kaidan(&networkFactories);


	//
	// The GUI
	//

#ifdef SFOS // SailfishOS App

	// create an sailfish app + view
	QGuiApplication *app = SailfishApp::application(argc, argv);
	QQuickView *view = SailfishApp::createView();

	// connect the back-end class
	view->rootContext()->setContextProperty("kaidan", &kaidan);

	// load the main qml-file and show it in fullscreen
	view->setSource(QUrl::fromLocalFile("/usr/share/harbour-kaidan/qml/main.qml"));
	view->showFullScreen();

	// run the app
	return app->exec();

#else // normal Qt App

	// create an app
	QGuiApplication app(argc, argv);
	// initialize the qml engine
	QQmlApplicationEngine engine;

	// connect the back-end class
	engine.rootContext()->setContextProperty("kaidan", &kaidan);
	// load the main qml-file
	engine.load(QUrl("qrc:/qml/main.qml"));

	// create the qtquick window
	QObject *topLevel = engine.rootObjects().value(0);
	QQuickWindow *window = qobject_cast<QQuickWindow*>(topLevel);

	// show the window and run the app
	window->show();
	return app.exec();

#endif
}
