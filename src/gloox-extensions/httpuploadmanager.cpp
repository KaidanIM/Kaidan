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

#include "httpuploadmanager.h"
#include "httpuploadhandler.h"
#include "httpuploadrequest.h"
#include "httpuploadslot.h"
#include "httpuploader.h"
#include <gloox/dataform.h>
#include <gloox/dataformitem.h>
#include <gloox/dataformreported.h>
#include <gloox/error.h>

#include <fstream>
#include <sys/stat.h>

using namespace gloox;

HttpUploadManager::HttpUploadManager(Client *client)
	: client(client)
{
}

HttpUploadManager::~HttpUploadManager()
{
	client->removeIDHandler(this);
}

bool HttpUploadManager::valid()
{
	return uploader && handler && uploadServices.size() > 0;
}

int HttpUploadManager::uploadFile(std::string &path, bool queue,
                                  std::string contentType, std::string name,
                                  unsigned long length)
{
	if (!valid() || !existsFile(path))
		return -1;

	FileMeta *file = new FileMeta;
	file->id = newUploadId();
	file->path = path;
	file->name = name.length() == 0 ? getFileName(path) : name;
	file->contentType = contentType;
	file->length = length == 0 ? calculateFileSize(path) : length;
	file->state = UploadWaiting; // waiting in queue

	uploadQueue[file->id] = file;

	// if uploader allows, directly start upload
	if (!uploader->busy() || (!queue && uploader->supportsParallel())) {
		// directly start
		file->state = UploadRequested;
		sendUploadRequest(file->id, file->name, file->length, file->contentType);
	}
	return file->id;
}

void HttpUploadManager::tryAddFileStoreService(const JID &jid,
                                               const Disco::Info &info)
{
	// check http file upload feature
	if (!info.hasFeature(XMLNS_HTTPUPLOAD))
		return;

	// gloox can only get the first dataform of a Disco::Info result, thus we
	// need to check both the old and the new xml namespace of HTTP File Upload
	const DataForm *form = info.form();
	if (
		form->type() == TypeResult &&
		form->hasField("FORM_TYPE") &&
		(form->field("FORM_TYPE")->value() == XMLNS_HTTPUPLOAD ||
		 form->field("FORM_TYPE")->value() == "urn:xmpp:http:upload") &&
		form->hasField("max-file-size")
	) {
		// get old maximum file size
		unsigned long oldMaxFileSize = 0;
		for (UploadService &service : uploadServices) {
			if (service.maxFileSize > oldMaxFileSize)
				oldMaxFileSize = service.maxFileSize;
		}

		try {
			// get max file size for uploads from form data
			unsigned long maxFileSize = std::stoul(
				form->field("max-file-size")->value());

			// register new upload service
			addUploadService(jid, maxFileSize);
			// handle new upload service added
			handler->handleUploadServiceAdded(jid, maxFileSize);

			// handle new file size limit
			if (maxFileSize > oldMaxFileSize)
				handler->handleFileSizeLimitChanged(maxFileSize);
		} catch (std::invalid_argument &e) {
			// Couldn't parse data form field max-file-size.
		} catch (std::out_of_range &e) {
			// Data form field max-file-size is out of range of an unsigned long.
		}
	}
}

void HttpUploadManager::addUploadService(JID jid, const unsigned long maxFileSize)
{
	// add the new upload service to the list (if not existant already) and sort the list
	std::string bare = jid.bare();
	if (!hasUploadService(bare)) {
		UploadService service;
		service.jid = bare;
		service.maxFileSize = maxFileSize;

		uploadServices.emplace_back(service);
		sortUploadServices();
	}

	if (!uploader->busy())
		startNextUpload();
}

bool HttpUploadManager::hasUploadService(std::string &jid) const
{
	for (UploadService service : uploadServices) {
		if (service.jid == jid)
			return true;
	}
	return false;
}

void HttpUploadManager::sortUploadServices()
{
	// this will sort the upload services after the max file size (largest first)
	uploadServices.sort([] (const UploadService &a, const UploadService &b) {
		return a.maxFileSize > b.maxFileSize;
	});
}

void HttpUploadManager::sendUploadRequest(int id, std::string &filename,
                                          unsigned long size, std::string &contentType)
{
	JID uploadServiceJid = JID(uploadServices.front().jid);
	IQ requestIq(IQ::Get, uploadServiceJid, client->getID());

	HttpUploadRequest *requestExt = new HttpUploadRequest(filename, size, contentType);
	requestIq.addExtension(requestExt);

	client->send(requestIq, this, id, false);
}

void HttpUploadManager::handleIqID(const gloox::IQ &iq, int context)
{
	HttpUploadSlot *slot = (HttpUploadSlot*) iq.findExtension(EXT_HTTPUPLOADSLOT);
	if (slot && slot->valid()) {
		// update state of upload to in progress
		uploadQueue[context]->state = UploadInProgress;
		uploadQueue[context]->getUrl = slot->getUrl();

		uploader->uploadFile(
			context, slot->putUrl(), slot->putHeaderFields(),
			uploadQueue[context]->path
		);
	} else {
		HttpUploadRequest *request = (HttpUploadRequest*) iq.findExtension(EXT_HTTPUPLOADREQUEST);
		Error *error = (Error*) iq.findExtension(ExtError);
		HttpUploadError uplError;
		std::string stamp;

		// if there's no slot and no request or no error, an unexpected error occured
		if (!request || !error) {
			uplError = UploadUnknownError;
		} else if (error->type() == StanzaErrorTypeModify &&
		           error->error() == StanzaErrorNotAcceptable) {
			uplError = UploadTooLarge;
		} else if (error->type() == StanzaErrorTypeWait &&
		           error->error() == StanzaErrorResourceConstraint) {
			uplError = UploadQuotaReached;
			Tag *retry = error->tag()->findChild("retry", "xmlns", XMLNS_HTTPUPLOAD);
			if (retry)
				std::string stamp = retry->findAttribute("stamp");
		} else if (error->type() == StanzaErrorTypeCancel &&
		           error->error() == StanzaErrorNotAllowed) {
			uplError = UploadNotAllowed;

			// remove this upload service, if we aren't permitted to use it
			for (std::list<UploadService>::iterator it = uploadServices.begin();
			     it != uploadServices.end();) {
				if (it->jid == iq.from().bare())
					it = uploadServices.erase(it);
				else
					it++;
			}
		}

		// get text message of error
		std::string text = error->text();

		handler->handleUploadFailed(context, uplError, text, stamp);
		uploadQueue.erase(context);
		startNextUpload();
	}
}

void HttpUploadManager::uploadFinished(int id)
{
	std::string getUrl = uploadQueue[id]->getUrl;
	uploadQueue.erase(id);

	handler->handleUploadFinished(id, getUrl);
	startNextUpload();
}

void HttpUploadManager::uploadFailed(int id, HttpUploadError error)
{
	handler->handleUploadFailed(id, error);
	uploadQueue.erase(id);
}

void HttpUploadManager::uploadProgress(int id, unsigned long sent, unsigned long total)
{
	// if total upload size is not set, use length of local file
	handler->handleUploadProcess(id, sent, total != 0 ? total :
	                             uploadQueue[id]->length);
}

void HttpUploadManager::startNextUpload()
{
	for (const auto &pair : uploadQueue) {
		if (pair.second->state == UploadWaiting) {
			pair.second->state = UploadRequested;
			sendUploadRequest(pair.second->id, pair.second->name,
			                  pair.second->length, pair.second->contentType);
			break;
		}
	}
}

const unsigned long HttpUploadManager::maxFileSize()
{
	unsigned long maxFileSize = 0;
	for (UploadService srv : uploadServices) {
		if (srv.maxFileSize > maxFileSize)
			maxFileSize = srv.maxFileSize;
	}

	return maxFileSize;
}

bool HttpUploadManager::existsFile(const std::string &path) const
{
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

const unsigned long HttpUploadManager::calculateFileSize(std::string &path) const
{
	try {
		std::ifstream in(path.c_str(), std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	} catch (std::ios_base::failure &e) {
		return 0;
	}
}

const std::string HttpUploadManager::getFileName(std::string &path) const
{
#ifdef _WIN32
	char sep = '\\';
#else
	char sep = '/';
#endif

	size_t i = path.rfind(sep, path.length());
	if (i != std::string::npos) {
		return path.substr(i + 1, path.length() - i);
	}
	return "";
}
