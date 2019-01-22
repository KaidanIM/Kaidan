/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

#ifndef UPLOADMANAGER_H
#define UPLOADMANAGER_H

// QXmpp
#include "qxmpp-exts/QXmppUploadManager.h"
// Qt
#include <QObject>
#include <QMap>
#include <QSize>
// Kaidan
#include "Enums.h"
#include "MessageModel.h"

using namespace Enums;

class Kaidan;
class RosterManager;

/**
 * @class UploadManager Class for handling and starting HTTP File Uploads
 */
class UploadManager : public QObject
{
	Q_OBJECT

public:
	/**
	 * Default constructor
	 */
	UploadManager(Kaidan *kaidan, QXmppClient *client, MessageModel *msgModel,
	              RosterManager *rosterManager, QObject *parent = nullptr);

signals:
	/**
	 * Connect to it to be notified about progress on uploads
	 */
	void uploadProgressMade(QString msgId, unsigned long sent, unsigned long total);

public slots:
	/**
	 * Starts uploading a file
	 */
	void sendFile(QString jid, QString filePath, QString message);

	void handleUploadFailed(const QXmppHttpUpload *upload);
	void handleUploadProgressed(const QXmppHttpUpload *upload);
	void handleUploadSucceeded(const QXmppHttpUpload *upload);

private:
	Kaidan *kaidan;
	QXmppClient *client;
	QXmppUploadManager manager;
	MessageModel *msgModel;
	RosterManager *rosterManager;

	QMap<int, MessageModel::Message*> messages;
};

#endif // UPLOADMANAGER_H
