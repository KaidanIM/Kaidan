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

#include "jinglefile.h"
#include "gloox-extensions.h"
#include <gloox/util.h>
#include <QString>

using namespace gloox;
using namespace gloox::Jingle;

File::File(const std::string& name, long size, HashList hashes,
           const std::string& mediaType, const std::string& date,
           const std::string& desc, const Thumb* thumb, long offset, long length)
	: StanzaExtension(EXT_JINGLE_FILE), m_name(name), m_size(size),
	m_hashes(hashes), m_mediaType(mediaType), m_date(date), m_desc(desc),
	m_thumb(thumb), m_offset(offset), m_length(length), m_valid(true)
{
}

File::File(const Tag* tag)
	: StanzaExtension(EXT_JINGLE_FILE), m_valid(false), m_size(-1),
	m_offset(-1), m_length(-1)
{
	if (tag->name() == "file")
		m_valid = true;

	if (tag->hasChild("name"))
		m_name = tag->findChild("name")->cdata();
	if (tag->hasChild("media-type"))
		m_mediaType = tag->findChild("media-type")->cdata();
	if (tag->hasChild("desc"))
		m_desc = tag->findChild("desc")->cdata();
	if (tag->hasChild("date"))
		m_date = tag->findChild("date")->cdata();
	if (tag->hasChild("size")) {
		try {
			// conversion can cause invalid_argument / out_of_range exception
			m_size = QString::fromStdString(tag->findChild("size")->cdata()).toLong();
		} catch (std::invalid_argument &e) {
			// Couldn't parse size: input probably doesn't contain valid number
		} catch (std::out_of_range &e) {
			// Couldn't parse size: is out of range of an unsigned long
		}
	}

	if (tag->hasChild("range")) {
		Tag* range = tag->findChild("range");
		if (range->hasAttribute("offset")) {
			try {
				// conversion can cause invalid_argument / out_of_range exception
				m_offset = QString::fromStdString(tag->findAttribute("offset")).toLong();
			} catch (std::invalid_argument &e) {
				// Couldn't parse size: input probably doesn't contain valid number
			} catch (std::out_of_range &e) {
				// Couldn't parse size: is out of range of an unsigned long
			}
		}
		if (range->hasAttribute("length")) {
			try {
				// conversion can cause invalid_argument / out_of_range exception
				m_length = QString::fromStdString(tag->findAttribute("length")).toLong();
			} catch (std::invalid_argument &e) {
				// Couldn't parse size: input probably doesn't contain valid number
			} catch (std::out_of_range &e) {
				// Couldn't parse size: is out of range of an unsigned long
			}
		}
	}

	// a list of all children of type hash
	TagList hashes = tag->findChildren("hash");
	hashes.splice(hashes.end(), tag->findChildren("hash-used"));

	for (Tag* hash : hashes) {
		if (hash->xmlns() == XMLNS_HASHES_2) {
			Hash hashObj(hash);
			if (hashObj.valid())
				m_hashes.emplace_back(hashObj);
		}
	}

	m_thumb = new Thumb(tag->findChild("thumb", XMLNS, XMLNS_THUMBS));
}

Tag* File::tag() const
{
	// is *always* including the XMLNS acceptable? (only needed for non-jingle usages)
	Tag* tag = new Tag("file", XMLNS, XMLNS_JINGLE_FILE_TRANSFER_5);
	if (!m_name.empty()) {
		Tag* name = new Tag("name");
		name->addCData(m_name);
		tag->addChild(name);
	}
	if (!m_mediaType.empty()) {
		Tag* mediaType = new Tag("media-type");
		mediaType->addCData(m_mediaType);
		tag->addChild(mediaType);
	}
	if (!m_desc.empty()) {
		Tag* desc = new Tag("desc");
		desc->addCData(m_desc);
		tag->addChild(desc);
	}
	if (!m_date.empty()) {
		Tag* date = new Tag("date");
		date->addCData(m_date);
		tag->addChild(date);
	}
	if (m_size > 0) {
		Tag* size = new Tag("size");
		size->addCData(util::long2string(m_size));
		tag->addChild(size);
	}
	if (m_offset > 0) {
		Tag* range = new Tag("range");
		range->addAttribute("offset", m_offset);
		if (m_length > 0)
			range->addAttribute("length", m_length);
		tag->addChild(range);
	}
	if (!m_hashes.empty()) {
		for (Hash hash : m_hashes)
			tag->addChild(hash.tag());
	}
	if (m_thumb && m_thumb->valid())
		tag->addChild(m_thumb->tag());

	return tag;
}

const std::string& File::filterString() const
{
	static const std::string filter = "file";
	return filter;
}
