/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2018 Kaidan developers and contributors
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

#ifndef UPLOADHANDLER_H
#define UPLOADHANDLER_H

// gloox
#include "gloox-extensions/httpuploadhandler.h"
#include "gloox-extensions/httpuploadmanager.h"
// Qt
#include <QObject>
#include <QMap>
#include <QSize>
// Kaidan
#include "Enums.h"

using namespace Enums;

namespace gloox {
	class Client;
}

class MessageHandler;
class MessageModel;
class QtHttpUploader;

/**
 * @class UploadHandler Class for handling and starting HTTP File Uploads
 */
class UploadHandler : public QObject, public gloox::HttpUploadHandler
{
	Q_OBJECT

public:
	/**
	 * Default constructor
	 */
	UploadHandler(gloox::Client *client, MessageHandler *msgHandler,
	              MessageModel *msgModel, QObject *parent = nullptr);

	gloox::HttpUploadManager* getUploadManager()
	{
		return manager;
	}

signals:
	/**
	 * Connect to it to be notified about progress on uploads
	 */
	void uploadProgressMade(QString msgId, unsigned long sent, unsigned long total);

public slots:
	/**
	 * Starts uploading a file
	 */
	void uploadFile(QString jid, QString filePath, QString message);

protected:
	/**
	 * Called, when a new upload service was added.
	 *
	 * @param jid The JID of the upload service that has been added
	 * @param maxFileSize The maximum file size for uploading to this service
	 */
	virtual void handleUploadServiceAdded(const gloox::JID &jid,
	                                      unsigned long maxFileSize);

	/**
	 * Called, when an upload server has been removed.
	 *
	 * @param jid The JID of the upload service that has been removed
	 */
	virtual void handleUploadServiceRemoved(const gloox::JID &jid);

	/**
	 * Called, when the file size limit has changed.
	 *
	 * @param maxFileSize The new maximum file size for uploading
	 */
	virtual void handleFileSizeLimitChanged(unsigned long maxFileSize);

	/**
	 * Called, when the uploader made progress
	 *
	 * @param id Upload job id
	 * @param sent Number of bytes that has been sent
	 * @param total Number of total bytes to upload
	 */
	virtual void handleUploadProcess(int id, unsigned long sent,
	                                 unsigned long total);

	/**
	 * Called, when an upload has successfully finished
	 *
	 * @param id Upload job id
	 * @param getUrl HTTPS GET url to share with others and download the file
	 */
	virtual void handleUploadFinished(int id, std::string &name,
	                                  std::string &getUrl, std::string &contentType,
	                                  unsigned long &size);

	/**
	 * Called, when an upload job has failed
	 *
	 * @param id Upload job id
	 * @param error The error that has occured
	 * @param text An optional message about what went wrong
	 */
	virtual void handleUploadFailed(int id, gloox::HttpUploadError error,
	                                const std::string &text = gloox::EmptyString,
	                                const std::string &stamp = gloox::EmptyString);

private:
	/**
	 * Generates a media thumbnail (currently only image thumbs)
	 */
	QSize generateMediaThumb(QString &filePath, MessageType type,
	                        QByteArray *bytes);

	struct MediaSharingMeta {
		QString jid;
		std::string msgId;
		QString message;
		QString filePath;
		MessageType type;
	};

	gloox::Client *client;
	gloox::HttpUploadManager *manager;
	MessageHandler *msgHandler;
	MessageModel *msgModel;
	QtHttpUploader *uploader;

	QMap<int, MediaSharingMeta> mediaShares;
};

#endif // UPLOADHANDLER_H
