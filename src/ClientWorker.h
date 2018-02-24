/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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

#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

// Qt
#include <QObject>
#include <QTimer>
// gloox
#include <gloox/connectionlistener.h>

class ClientThread;
class GlooxClient;
class QGuiApplication;

/**
 * The ClientWorker is used as a QObject-based worker on the ClientThread.
 * 
 * @see ClientThread
 */
class ClientWorker : public QObject, public gloox::ConnectionListener
{
	Q_OBJECT

public:
	/**
	 * @param client XMPP Client which will be worked on.
	 * @param controller The ClientThread instance for emitting signals.
	 * @param parent Optional QObject-based parent.
	 */
	ClientWorker(GlooxClient *client, ClientThread *contoller,
	             QGuiApplication *app, QObject *parent = nullptr);

	~ClientWorker();

signals:
	void stopReconnectTimerRequested();

public slots:
	/**
	 * Main function that will be executed via a QTimer.
	 * 
	 * If the client is not disconnected and not untouched, this will fetch
	 * new packages from the client and process them.
	 */
	void updateClient();

	/**
	 * Connects the client with the server.
	 */
	void xmppConnect();

	/**
	 * Disconnects the client from server.
	 */
	void xmppDisconnect();

	/**
	 * Destruct timer used for client loop (needs to be killed on the client
	 * thread)
	 */
	void stopWorkTimer();

	/**
	 * Sets the application state used for XEP-0352: Client State Indication
	 */
	void setApplicationState(Qt::ApplicationState state);

private:
	/**
	 * Notifys via signal that the client has connected.
	 */
	virtual void onConnect();

	/**
	 * Saves error reason and emits a disconnect signal.
	 */
	virtual void onDisconnect(gloox::ConnectionError error);

	/**
	 * Always accepts the TLS certificate
	 * 
	 * @todo Automatically check if certificate is valid and ask user.
	 */
	virtual bool onTLSConnect(const gloox::CertInfo &info);

	/**
	 * Start timer to reconnect after connection failed
	 */
	void reconnect();

	GlooxClient *client;
	ClientThread *controller;
	QTimer reconnectTimer;
};

#endif // CLIENTWORKER_H
