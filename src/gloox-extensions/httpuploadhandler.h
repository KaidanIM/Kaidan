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

#ifndef HTTPUPLOADHANDLER_H__
#define HTTPUPLOADHANDLER_H__

#include <gloox/jid.h>
#include "httpuploadmanager.h"

#include <string.h>

namespace gloox {
	class HttpUploadSlot;

	/**
	 * @class HttpUploadHandler A virtual interface that enables objects to
	 * receive HTTP File Upload (@xep{0363}) events.
	 *
	 * It can be registered at the @c HttpUploadManager.
	 *
	 * XEP Version: 0.5
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API HttpUploadHandler
	{
	public:
		/**
		 * Called, when a new upload service was added.
		 *
		 * @param jid The JID of the upload service that has been added
		 * @param maxFileSize The maximum file size for uploading to this service
		 */
		virtual void handleUploadServiceAdded(const JID &jid,
		                                      unsigned long maxFileSize) = 0;

		/**
		 * Called, when an upload server has been removed.
		 *
		 * @param jid The JID of the upload service that has been removed
		 */
		virtual void handleUploadServiceRemoved(const JID &jid) = 0;

		/**
		 * Called, when the file size limit has changed.
		 *
		 * @param maxFileSize The new maximum file size for uploading
		 */
		virtual void handleFileSizeLimitChanged(unsigned long maxFileSize) = 0;

		/**
		 * Called, when the uploader made progress
		 *
		 * @param id Upload job id
		 * @param sent Number of bytes that has been sent
		 * @param total Number of total bytes to upload
		 */
		virtual void handleUploadProcess(int id, unsigned long sent,
		                                 unsigned long total) = 0;

		/**
		 * Called, when an upload has successfully finished
		 *
		 * @param id Upload job id
		 * @param name The file name that was used for the file upload
		 * @param getUrl HTTPS GET url to share with others and download the file
		 * @param contentType The MIME-type of the file (can be empty)
		 * @param length The file size in bytes
		 */
		virtual void handleUploadFinished(int id, std::string& name,
		                                  std::string& getUrl, std::string& contentType,
		                                  unsigned long& length) = 0;

		/**
		 * Called, when an upload job has failed
		 *
		 * @param id Upload job id
		 * @param error The error that has occured
		 * @param text An optional message about what went wrong
		 * @param stamp A UTC timestamp that will show the date, when the next
		 * upload can be made, if the upload quota was reached.
		 */
		virtual void handleUploadFailed(int id, HttpUploadError error,
		                                const std::string &text = EmptyString,
		                                const std::string &stamp = EmptyString) = 0;
	};
}

#endif // HTTPUPLOADHANDLER_H__
