/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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
#include <QThread>
#include <QSettings>
class QGuiApplication;
// QXmpp
#include <QXmppClient.h>
// Kaidan
#include "Globals.h"
#include "Database.h"
#include "MessageModel.h"
#include "RosterModel.h"
#include "AvatarFileStorage.h"
#include "PresenceCache.h"
class Kaidan;
class ClientWorker;
class RosterManager;

class ClientThread : public QThread
{
	Q_OBJECT
	friend ClientWorker;

public:
	ClientThread()
	{
		setObjectName("QXmppClient");
	}

protected:
	void run() override
	{
		exec();
	}
};

/**
 * The ClientWorker is used as a QObject-based worker on the ClientThread.
 */
class ClientWorker : public QObject
{
	Q_OBJECT

public:
	struct Caches {
		Caches(Database *database)
			: msgModel(new MessageModel(database->getDatabase())),
			  rosterModel(new RosterModel(database->getDatabase())),
			  avatarStorage(new AvatarFileStorage()),
			  presCache(new PresenceCache()),
			  settings(new QSettings(APPLICATION_NAME, APPLICATION_NAME))
		{
		}

		~Caches()
		{
			delete msgModel;
			delete rosterModel;
			delete avatarStorage;
			delete presCache;
			delete settings;
		}

		MessageModel *msgModel;
		RosterModel *rosterModel;
		AvatarFileStorage *avatarStorage;
		PresenceCache *presCache;
		QSettings *settings;
	};

	struct Credentials {
		QString jid;
		QString jidResource;
		QString password;
		// if never connected successfully before with these credentials
		bool isFirstTry;
	};

	/**
	 * @param caches All caches running in the main thread for communication with the UI.
	 * @param kaidan Main back-end class, running in the main thread.
	 * @param enableLogging If logging of the XMPP stream should be done.
	 * @param app The QGuiApplication to determine if the window is active.
	 * @param parent Optional QObject-based parent.
	 */
	ClientWorker(Caches *caches, Kaidan *kaidan, bool enableLogging, QGuiApplication *app,
	             QObject *parent = nullptr);

	~ClientWorker();

public slots:
	/**
	 * Main function of the client thread
	 */
	void main();

	/**
	 * Sets the new credentials for next connect.
	 * 
	 * @param creds The new credentials for the next connect
	 */
	void setCredentials(Credentials creds)
	{
		this->creds = creds;
	}

	/**
	 * Connects the client with the server.
	 */
	void xmppConnect();

signals:
	void connectRequested();
	void disconnectRequested();
	void credentialsUpdated(Credentials creds);
	void sendMessageRequested(QString jid, QString message);
	void sendFileRequested(QString jid, QString filePath, QString message);
	void addContactRequested(QString jid, QString nick, QString message);
	void removeContactRequested(QString jid);

private slots:
	/**
	 * Notifys via signal that the client has connected.
	 */
	void onConnect();

	/**
	 * Shows error reason
	 */
	void onConnectionError(QXmppClient::Error error);

private:
	/**
	 * Generates a random alphanumeric string
	 *
	 * @param length The length of the generated string
	 */
	QString generateRandomString(unsigned int length = 4) const;

	Caches *caches;
	Kaidan *kaidan;
	QXmppClient *client;
	Credentials creds;
	bool enableLogging;
	QGuiApplication *app;

	RosterManager *rosterManager;
};

#endif // CLIENTWORKER_H
