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

#include "httpuploadrequest.h"
#include "gloox-extensions.h"
#include <gloox/tag.h>

using namespace gloox;

HttpUploadRequest::HttpUploadRequest()
	: StanzaExtension(EXT_HTTPUPLOADSLOT), m_filename(""), m_contentType(""),
	m_size(0), m_valid(false)
{
}

HttpUploadRequest::HttpUploadRequest(const std::string &filename, const unsigned long &size,
	const std::string &contentType) : StanzaExtension(EXT_HTTPUPLOADREQUEST),
	m_filename(filename), m_size(size), m_contentType(contentType)
{
	m_valid = !m_filename.empty() && m_size;
}

HttpUploadRequest::HttpUploadRequest(const Tag *tag) : gloox::StanzaExtension(EXT_HTTPUPLOADREQUEST)
{
	m_valid = false;
	if (
		tag &&
		tag->name() == "request" &&
		tag->hasAttribute(XMLNS, XMLNS_HTTPUPLOAD) &&
		tag->hasAttribute("filename") &&
		tag->hasAttribute("size")
	) {
		m_filename = tag->findAttribute("filename");
		m_contentType = tag->findAttribute("contentType");
		try {
			// conversion can cause invalid_argument / out_of_range exception
			m_size = std::stoul(tag->findAttribute("size"));
			m_valid = true;
		} catch (std::invalid_argument &e) {
			// Couldn't parse size: input probably doesn't contain valid number
		} catch (std::out_of_range &e) {
			// Couldn't parse size: is out of range of an unsigned long
		}
	}
}

HttpUploadRequest::~HttpUploadRequest()
{
}

const std::string& HttpUploadRequest::filterString() const
{
	static const std::string filter = "/iq/request[@xmlns='" +
	                                  XMLNS_HTTPUPLOAD + "']";
	return filter;
}

Tag* HttpUploadRequest::tag() const
{
	Tag *tag = new Tag("request", XMLNS, XMLNS_HTTPUPLOAD);
	tag->addAttribute("filename", m_filename);
	tag->addAttribute("size", std::to_string(m_size));
	if (!m_contentType.empty())
		tag->addAttribute("content-type", m_contentType);

	return tag;
}
