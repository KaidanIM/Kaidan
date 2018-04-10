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

#ifndef GLOOXEXTS_H__
#define GLOOXEXTS_H__

#include <string>
#include <map>

namespace gloox {
	//
	// XEP-0363: HTTP File Upload
	//

	static const std::string XMLNS_HTTPUPLOAD = "urn:xmpp:http:upload:0";

	// random number above normal StanzaExtensions
	static const int EXT_HTTPUPLOADREQUEST = 0x10AD; //=> 4269
	static const int EXT_HTTPUPLOADSLOT = 4270;

	typedef std::map<std::string, std::string> HeaderFieldMap;

	enum HttpUploadState {
		UploadNone,             /* No state */
		UploadWaiting,          /* Upload is waiting in queue */
		UploadRequested,        /* An upload slot has been requested */
		UploadInProgress        /* The upload is in progress */
	};

	enum HttpUploadError {
		UploadNoError,           /* No error occured */
		UploadUnknownError,      /* An unknown error occured */
		UploadTooLarge,          /* The requested file is too large */
		UploadQuotaReached,      /* The upload quota was reached, try again later */
		UploadNotAllowed,        /* The upload service doesn't allow uploads from us */
		UploadFileNotFound,      /* Couldn't find or read file on disk */
		UploadHttpError          /* Error occured while uploading via HTTPS */
	};

	//
	// XEP-0231: Bits of Binary
	//

	static const std::string XMLNS_BITSOFBINARY = "urn:xmpp:bob";
	static const int EXT_BITSOFBINARY = 4271;

	//
	// XEP-0372: References
	//

	static const std::string XMLNS_REFERENCES = "urn:xmpp:reference:0";
	static const int EXT_REFERENCES = 4272;

	//
	// XEP-0334: Message Processing Hints
	//

	static const std::string XMLNS_PROCESSINGHINTS = "urn:xmpp:hints";
	static const int EXT_PROCESSINGHINTS = 4273;

	//
	// XEP-0300: Use of Cryptographic Hash functions in XMPP
	//

	static const std::string XMLNS_HASHES_2 = "urn:xmpp:hashes:2";
	static const int EXT_HASHES = 4274;

	static const std::string XMLNS_HASH_MD5 = "urn:xmpp:hash-function-text-names:md5";
	static const std::string XMLNS_HASH_SHA_1 = "urn:xmpp:hash-function-text-names:sha-1";
	static const std::string XMLNS_HASH_SHA_224 = "urn:xmpp:hash-function-text-names:sha-224";
	static const std::string XMLNS_HASH_SHA_256 = "urn:xmpp:hash-function-text-names:sha-256";
	static const std::string XMLNS_HASH_SHA_384 = "urn:xmpp:hash-function-text-names:sha-384";
	static const std::string XMLNS_HASH_SHA_512 = "urn:xmpp:hash-function-text-names:sha-512";
	static const std::string XMLNS_HASH_SHA3_224 = "urn:xmpp:hash-function-text-names:sha3-224";
	static const std::string XMLNS_HASH_SHA3_256 = "urn:xmpp:hash-function-text-names:sha3-256";
	static const std::string XMLNS_HASH_SHA3_384 = "urn:xmpp:hash-function-text-names:sha3-384";
	static const std::string XMLNS_HASH_SHA3_512 = "urn:xmpp:hash-function-text-names:sha3-512";
	static const std::string XMLNS_HASH_BLAKE2B_160 = "urn:xmpp:hash-function-text-names:id-blake2b160";
	static const std::string XMLNS_HASH_BLAKE2B_256 = "urn:xmpp:hash-function-text-names:id-blake2b256";
	static const std::string XMLNS_HASH_BLAKE2B_384 = "urn:xmpp:hash-function-text-names:id-blake2b384";
	static const std::string XMLNS_HASH_BLAKE2B_512 = "urn:xmpp:hash-function-text-names:id-blake2b512";

	//
	// XEP-0264: Jingle Content Thumbnails
	//

	static const std::string XMLNS_THUMBS = "urn:xmpp:thumbs:1";
	static const int EXT_THUMBS = 4277;

	//
	// XEP-0234: Jingle File Transfer
	//

	static const std::string XMLNS_JINGLE_FILE_TRANSFER_5 = "urn:xmpp:jingle:apps:file-transfer:5";
	static const int EXT_JINGLE_FILE = 4275;

	//
	// XEP-0385: Stateless Inline Media Sharing (SIMS)
	//

	static const std::string XMLNS_SIMS = "urn:xmpp:sims:1";
	static const int EXT_SIMS = 4276;
}

#endif // GLOOXEXTS_H__
