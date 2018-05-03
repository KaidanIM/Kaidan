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

#ifndef HTTPUPLOADER_H__
#define HTTPUPLOADER_H__

#include <string>
#include <map>
#include <gloox/gloox.h>
#include "gloox-extensions.h"

namespace gloox {
	class HttpUploadManager;

	/**
	 * @class HttpUploader A virtual interface for implementing an HTTP Uploader
	 * that can be used with @c HttpUploadManager.
	 *
	 * This class is only used for uploading files from disk to the server.
	 *
	 * XEP Version: 0.5
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API HttpUploader
	{
	public:
		/**
		 * Uploads a file to an HTTP PUT url from a local path
		 */
		virtual void uploadFile(int id, std::string putUrl,
		                        HeaderFieldMap putHeaders,
		                        std::string& localPath,
		                        std::string& contentType) = 0;

		/**
		 * Returns true, if supporting multiple uploads at the same time
		 */
		virtual bool supportsParallel() = 0;

		/**
		 * Must return true, while uploading, even if parrallel uploads are
		 * supported
		 */
		virtual bool busy() = 0;
	};
}

#endif // HTTPUPLOADER_H__
