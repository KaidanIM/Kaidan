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

#ifndef HTTPUPLOADMANAGER_H__
#define HTTPUPLOADMANAGER_H__

#include <gloox/client.h>
#include <gloox/discohandler.h>
#include "gloox-extensions.h"
#include <string.h>
#include <list>
#include <map>

namespace gloox {
	class HttpUploadHandler;
	class HttpUploader;

	/**
	 * @class HttpUploadManager A manager that handles upload services and
	 * can upload files (@xep{0363}).
	 *
	 * First, you need to register the HTTP File Upload stanza extensions to
	 * make them usable in your gloox @c Client.
	 *
	 * @code
	 * client->registerStanzaExtension( new HttpUploadRequest() );
	 * client->registerStanzaExtension( new HttpUploadSlot() );
	 * @endcode
	 *
	 * The second step is to to create an @c HttpUploadManager and register an
	 * @c HttpUploader and an @c HttpUploadHandler to it. Gloox doesn't provide
	 * an ready to use @c HttpUploader for you, so unfortunately you need to
	 * implement one, first. After that we can create our @c HttpUploadHandler
	 * based class. Here's an example:
	 *
	 * @code
	 * MyUploadHandler::MyUploadHandler(Client *client)
	 * {
	 *   manager = new HttpUploadManager( client );
	 *   uploader = new MyHttpUploader( manager );
	 * 
	 *   manager->registerHttpUploadHandler( this );
	 *   manager->registerHttpUploader( uploader );
	 * }
	 * @endcode
	 *
	 * Before you can upload files, you need to find a valid upload service.
	 * This is done by running a service discovery (@xep{0030}). On connect
	 * we request the server's disco items. When they've arrived, we start an
	 * disco info request to each of those disco items, check their category
	 * and type and try to add them to our @c HttpUploadManager.
	 *
	 * In the example the MyDiscoManager is registered as @c ConnectionListener
	 * and as @c DiscoHandler.
	 *
	 * @code
	 * void MyDiscoManager::onConnect()
	 * {
	 *   // request disco items from the server
	 *   JID serverJid = JID( client->server() );
	 *   disco->getDiscoItems( serverJid, std::string(), this, 0 );
	 * }
	 *
	 * void MyDiscoManager::handleDiscoItems( const gloox::JID& from, const gloox::Disco::Items& items, int context )
	 * {
	 *   if ( from.bare() == client->server() ) {
	 *     // get all disco items of the server and check their disco info
	 *     for ( Disco::Item *item : items.items() ) {
	 *       disco->getDiscoInfo( item->jid(), std::string(), this, 0 );
	 *     }
	 *   }
	 * }
	 *
	 * void MyDiscoManager::handleDiscoInfo( const gloox::JID& from, const gloox::Disco::Info& info, int context )
	 * {
	 *   for ( Disco::Identity *identity : info.identities() ) {
	 *     // check identity for file storage
	 *     if ( identity->category() == "store" && identity->type() == "file" ) {
	 *       httpUploadManager->tryAddFileStoreService( from, info );
	 *       // break out of for loop to not add one service multiple times
	 *       break;
	 *     }
	 *   }
	 * }
	 * @endcode
	 *
	 * When we want to upload a file, we should first check if the
	 * @c HttpUploadManager is really valid already. In this example we would
	 * do that in our MyUploadHandler class.
	 * 
	 * @code
	 * void MyUploadHandler::uploadFile( std::string &path )
	 * {
	 *   if ( manager->valid() ) {
	 *     // get mime-type (can be omitted and has to be implemented by you)
	 *     std::string mimeType = getContentType( path );
	 *     // queues file upload (and starts it)
	 *     int uplId = manager->uploadFile( path, true, mimeType );
	 *     if ( uplId < 0 )
	 *       return false; // upload failed
	 *
	 *     // you can save the uplId to identify later events to the id (i.e.
	 *     // finished or upload progress)
	 *   }
	 * }
	 * @endcode
	 *
	 * XEP Version: 0.5
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API HttpUploadManager : public IqHandler
	{
	public:
		struct UploadService {
			std::string jid;
			unsigned long maxFileSize;
		};
		typedef std::list<UploadService> UploadServiceList;

		struct FileMeta {
			int id;
			std::string name;
			std::string path;
			std::string contentType;
			std::string getUrl;
			unsigned long length;
			HttpUploadState state;
		};
		typedef std::map<int, FileMeta*> FileMetaMap;

		HttpUploadManager(gloox::Client *client);
		~HttpUploadManager();

		void registerHttpUploadHandler(HttpUploadHandler *handler)
		{
			this->handler = handler;
		}

		void registerHttpUploader(HttpUploader *uploader)
		{
			this->uploader = uploader;
		}

		/**
		 * @return true, if the manager is able to upload files.
		 */
		bool valid();

		/**
		 * Checks Disco info for HTTP File Upload and adds it, if valid.
		 *
		 * @param service The JID of the service
		 * @param info The disco info of this service
		 */
		void tryAddFileStoreService(const gloox::JID &service,
		                            const gloox::Disco::Info &info);

		/**
		 * Add an HTTP File Upload service for uploading files to.
		 */
		void addUploadService(gloox::JID jid, const unsigned long maxFileSize);

		/**
		 * Will request the upload of a file and start the upload, all answers
		 * can be handled by the @see HttpUploadHandler.
		 *
		 * @param filename The filename that will be used to get the upload
		 * URL from the server.
		 * @param filepath The local path to the file used by the HttpUploader
		 * to read and upload the file.
		 * @param length The length of the file in bytes.
		 * @param queue If false, will upload this file in parallell to others,
		 * but only if supported by @c HttpUploader.
		 * @param contentType Optional MIME content type used for uploading.
		 *
		 * @return Returns -1, if the upload directly failed, otherwise
		 * the ID which will be used to track later events
		 */
		int uploadFile(std::string &path, bool queue = true,
		               std::string contentType = std::string(""),
		               std::string name = std::string(""),
		               unsigned long length = 0);

		/**
		 * Notifies handler that the file upload has finished and the get
		 * URL is ready to use, now. (for @c HttpUploader)
		 */
		void uploadFinished(int id);

		/**
		 * Notifies handler that the file upload has failed. (for @c HttpUploader)
		 */
		void uploadFailed(int id, HttpUploadError error);

		/**
		 * Notifies handler about the upload progress of a upload job. (for @c HttpUploader)
		 */
		void uploadProgress(int id, unsigned long sent, unsigned long total);

	protected:
		/**
		 * Handles incoming IQs (should actually not be used since we only
		 * send IQs with a context)
		 */
		virtual bool handleIq(const IQ &iq)
		{
			return false;
		}

		/**
		 * Handles incoming IQs to get upload links and errors messages
		 */
		virtual void handleIqID(const IQ &iq, int context);

	private:
		/**
		 * Check if file exists
		 * @return true, if file exists
		 */
		bool existsFile(const std::string &path) const;

		/**
		 * Calculates the file size in bytes of a local file
		 */
		const unsigned long calculateFileSize(std::string &filePath) const;

		/**
		 * Gets the file name (including extension) of a full file path
		 */
		const std::string getFileName(std::string &filePath) const;

		/**
		 * Creates a new upload ID
		 */
		int newUploadId()
		{
			// returns old next upload id and then increases it
			return nextUploadId++;
		}

		/**
		 * Gets the maximum file size available for uploading
		 */
		const unsigned long maxFileSize();

		/**
		 * Sends a request for the file upload to the server
		 */
		void sendUploadRequest(int id, std::string &filename, unsigned long size,
		                       std::string &contentType);

		/**
		 * Checks if upload service JID is already in list
		 */
		bool hasUploadService(std::string &jid) const;

		/**
		 * Sorts the list of upload services by max file upload size
		 */
		void sortUploadServices();

		/**
		 * Searches the upload queue for the next upload an starts it
		 */
		void startNextUpload();

		int nextUploadId = 1000;

		Client *client;
		HttpUploadHandler *handler;
		HttpUploader *uploader;
		UploadServiceList uploadServices;
		FileMetaMap uploadQueue;
	};
}

#endif // HTTPUPLOADMANAGER_H__
