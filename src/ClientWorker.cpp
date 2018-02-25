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

#include "ClientWorker.h"
// gloox
#include <gloox/client.h>
// Qt
#include <QDebug>
#include <QSettings>
#include <QGuiApplication>
// Kaidan
#include "ClientThread.h"

// interval in seconds in which a new connection will be tryed
static const unsigned int RECONNECT_INTERVAL = 5000;

ClientWorker::ClientWorker(GlooxClient* client, ClientThread *controller,
                           QGuiApplication *app, QObject* parent)
	: QObject(parent), client(client), controller(controller)
{
	client->registerConnectionListener(this);

	// reconnect timer
	reconnectTimer.moveToThread(controller);
	reconnectTimer.setInterval(RECONNECT_INTERVAL);
	reconnectTimer.setSingleShot(true); // only call once
	connect(&reconnectTimer, &QTimer::timeout, this, &ClientWorker::xmppConnect);
	connect(this, &ClientWorker::stopReconnectTimerRequested,
	        &reconnectTimer, &QTimer::stop);
	connect(app, &QGuiApplication::applicationStateChanged,
	        this, &ClientWorker::setApplicationState);
}

ClientWorker::~ClientWorker()
{
	emit stopReconnectTimerRequested();
}

void ClientWorker::updateClient()
{
	if (controller->connState != ConnectionState::StateNone && controller->connState !=
	    ConnectionState::StateDisconnected) {
		controller->mutex.lock();
		client->recv(0);
		controller->mutex.unlock();
	}
}

void ClientWorker::xmppConnect()
{
	qDebug() << "[client] Connecting...";
	controller->setConnectionState(ConnectionState::StateConnecting);
	controller->mutex.lock();
	client->connect(false);
	controller->mutex.unlock();
}

void ClientWorker::xmppDisconnect()
{
	qDebug() << "[client] Disconnecting...";
	if (controller->connState != ConnectionState::StateDisconnecting) {
		controller->setConnectionState(ConnectionState::StateDisconnecting);
		controller->mutex.lock();
		client->disconnect();
		controller->mutex.unlock();
	}
}

void ClientWorker::onConnect()
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Connected successfully to server";
	controller->setConnectionState(ConnectionState::StateConnected);

	// Emit signal, that logging in with these credentials has worked for the first time
	if (controller->creds.isFirstTry)
		emit controller->logInWorked();

	// accept credentials and save them
	controller->creds.isFirstTry = false;
	controller->settings->setValue("auth/jid", controller->creds.jid);
	controller->settings->setValue("auth/password", QString::fromUtf8(controller->creds.password.toUtf8().toBase64()));
}

void ClientWorker::onDisconnect(gloox::ConnectionError error)
{
	// no mutex needed, because this is called from updateClient()
	qDebug() << "[client] Disconnected:" << (DisconnReason) error;

	// update connection state and error
	controller->setConnectionError(error);
	controller->setConnectionState(ConnectionState::StateDisconnected);

	// Check if first time connecting with these credentials
	if (controller->creds.isFirstTry) {
		// always request new credentials, when failed to connect on first time
		emit controller->newCredentialsNeeded();
	} else {
		// already connected with these credentials once
		if (error == gloox::ConnAuthenticationFailed)
			// if JID/password is wrong, request new credentials from QML
			emit controller->newCredentialsNeeded();
		else if (error != gloox::ConnUserDisconnected)
			reconnect();
	}
}

bool ClientWorker::onTLSConnect(const gloox::CertInfo &info)
{
	// no mutex needed, because this is called from updateClient()
	// accept certificate
	qDebug() << "[client] Accepted TLS certificate without checking";
	return true;
}

void ClientWorker::stopWorkTimer()
{
	controller->workTimer.stop();
}

void ClientWorker::reconnect()
{
	qDebug().noquote() << QString("[client] Will do reconnect in %1 ms").arg(RECONNECT_INTERVAL);
	reconnectTimer.start();
}

void ClientWorker::setApplicationState(Qt::ApplicationState state)
{
	if (state == Qt::ApplicationActive)
		controller->client->setActive();
	else
		controller->client->setInactive();
}
