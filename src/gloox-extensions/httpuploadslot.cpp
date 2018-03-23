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

#include "httpuploadslot.h"
#include <algorithm>

using namespace gloox;

HttpUploadSlot::HttpUploadSlot()
	: StanzaExtension(EXT_HTTPUPLOADSLOT), m_putUrl(""), m_putHeader(),
	m_getUrl(""), m_valid(false)
{
}

HttpUploadSlot::HttpUploadSlot(std::string &putUrl, std::string &getUrl,
                               std::map<std::string, std::string> &putHeader)
	: StanzaExtension(EXT_HTTPUPLOADSLOT), m_putUrl(putUrl), m_getUrl(getUrl),
	m_putHeader(putHeader)
{
	m_valid = !m_putUrl.empty() && !m_getUrl.empty();
}

HttpUploadSlot::HttpUploadSlot(const Tag *tag) : StanzaExtension(EXT_HTTPUPLOADSLOT)
{
	// check if correct tag
	if (!tag || tag->name() != "slot" || !tag->hasAttribute(XMLNS, XMLNS_HTTPUPLOAD))
		return;
	
	// get children 'put' and 'get'
	Tag *putTag = tag->findChild("put");
	Tag *getTag = tag->findChild("get");
	if (!putTag || !getTag)
		return;

	// get urls of put and get tags
	m_putUrl = putTag->findAttribute("url");
	m_getUrl = getTag->findAttribute("url");
	if (m_putUrl.empty() || m_getUrl.empty())
		return;

	// get all http put header fields and add them to a key-value map
	HeaderFieldMap m_putHeader;
	std::list<Tag*> headerList = putTag->findChildren("header");
	for (Tag *headerTag : headerList) {
		std::string key = headerTag->findAttribute("name");
		std::string value = headerTag->cdata();
		// don't add to map, if key or value is empty
		if (key.empty() || value.empty())
			continue;

		// check if key is valid
		// only "Authorization", "Cookie", "Expires" are allowed
		bool keyValid = false;
		for (const std::string &k : {"Authorization", "Cookie", "Expires"}) {
			if (key == k)
				keyValid = true;
		}
		if (!keyValid)
			continue;

		// replace newlines in the header fields
		while (value.find ("\r\n") != std::string::npos)
			value.erase(value.find("\r\n"), 2);
		while (value.find ("\n") != std::string::npos)
			value.erase(value.find("\n"), 1);

		m_putHeader[key] = value;
	}

	// check that URLs are HTTPS
	std::string prefix = "https://";
	for (const std::string &url : {m_putUrl, m_getUrl}) {
		if (url.compare(0, prefix.size(), prefix))
			return;
	}

	m_valid = true;
}

HttpUploadSlot::~HttpUploadSlot()
{
}

const std::string& HttpUploadSlot::filterString() const
{
	static const std::string filter = "/iq[@type='result']/slot[@xmlns='" +
		XMLNS_HTTPUPLOAD + "']";
	return filter;
}

gloox::Tag* HttpUploadSlot::tag() const
{
	Tag *slotTag = new Tag("slot", XMLNS, XMLNS_HTTPUPLOAD);
	// if not valid, return empty slot tag
	if (!m_valid)
		return slotTag;

	Tag *getTag = new Tag("get");
	getTag->addAttribute("url", m_getUrl);

	Tag *putTag = new Tag("put");
	for (auto& keyValue : m_putHeader) {
		Tag *headerField = new Tag("header");
		headerField->addAttribute("name", keyValue.first);
		headerField->setCData(keyValue.second);

		putTag->addChild(headerField);
	}

	// add child tags to the main tag
	slotTag->addChild(getTag);
	slotTag->addChild(putTag);

	return slotTag;
}
