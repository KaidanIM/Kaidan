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

#ifndef QTHTTPUPLOADER_H
#define QTHTTPUPLOADER_H

#include <string.h>
#include "gloox-extensions/httpuploader.h"
#include <QObject>
#include <QFile>
#include <QCryptographicHash>
#include <QMap>

/**
 * @class UploadFile Modified QFile for doing uploading and hashing, without
 * reading multple times
 */
class UploadFile : public QFile
{
	Q_OBJECT

public:
	UploadFile(QString name, QObject *parent = nullptr);

	QByteArray read(qint64 maxSize);

signals:
	void bytesRead(const QByteArray &bytes);
};


/**
 * @class QtHttpUploader Implementation of an HTTP File Uploader in Qt
 */
class QtHttpUploader : public QObject, public gloox::HttpUploader
{
	Q_OBJECT

public:
	struct HashResult {
		QByteArray sha256;
		QByteArray sha3_256;
	};

	/**
	 * Will register this uploader to the HttpUploadManager
	 *
	 * @param manager The HttpUploadManager this uploader will be used with
	 */
	QtHttpUploader(gloox::HttpUploadManager *manager, QObject *parent = nullptr);

	~QtHttpUploader();

	/**
	 * @return True, if currently uploading a file
	 */
	virtual bool busy();

	/**
	 * It does supports parallel uploads
	 */
	virtual bool supportsParallel()
	{
		return true;
	}

	/**
	 * @see gloox::HttpUploader::uploadFile
	 */
	virtual void uploadFile(int id, std::string putUrl,
	                        gloox::HeaderFieldMap putHeaders,
	                        std::string &localPath, std::string &contentType);

	/**
	 * Get all generated checksums of a file upload
	 */
	HashResult getHashResults(int id) const;

private:
	struct HashData {
		HashData()
			: sha256(new QCryptographicHash(QCryptographicHash::Sha256)),
			sha3_256(new QCryptographicHash(QCryptographicHash::Sha3_256))
		{
		};

		QCryptographicHash *sha256;
		QCryptographicHash *sha3_256;
	};

	void processHashes(int id, const QByteArray &bytes);

	gloox::HttpUploadManager *manager;
	unsigned int runningTasks = 0;
	QMap<int, HashData> hashCache;
};

#endif // QTHTTPUPLOADER_H
