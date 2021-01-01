/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

#pragma once

// Qt
#include <QObject>
#include <QMap>
// QXmpp
#include "qxmpp-exts/QXmppUploadManager.h"

class Message;
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
	UploadManager(QXmppClient *client, RosterManager* rosterManager,
	              QObject* parent = nullptr);

signals:
	/**
	 * Connect to it to be notified about progress on uploads
	 */
	void uploadProgressMade(QString msgId, unsigned long sent, unsigned long total);

public slots:
	/**
	 * Starts uploading a file
	 */
	void sendFile(const QString &jid, const QUrl &fileUrl, const QString &body);

	void handleUploadFailed(const QXmppHttpUpload *upload);
	void handleUploadSucceeded(const QXmppHttpUpload *upload);

private:
	QXmppClient *m_client;
	QXmppUploadManager m_manager;
	RosterManager *m_rosterManager;

	QMap<int, Message*> m_messages;
};
