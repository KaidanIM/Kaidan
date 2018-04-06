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

#include "reference.h"
#include "gloox-extensions.h"
#include "sims.h"
#include <gloox/tag.h>

//FIXME: remove
#include <iostream>

using namespace gloox;

Reference::Reference(ReferenceType type, int begin, int end,
                     std::string uri, std::string anchor)
	: StanzaExtension(EXT_REFERENCES), m_type(type), m_uri(uri),
	m_anchor(anchor), m_begin(begin), m_end(end), m_simsExt(nullptr)
{
	m_valid = type != ReferenceType::None;
	m_simsExt = nullptr;
}

Reference::Reference(ReferenceType type, std::string& uri)
	: StanzaExtension(EXT_REFERENCES), m_type(type), m_uri(uri), m_anchor(""),
	m_begin(-1), m_end(-1), m_simsExt(nullptr)
{
	m_valid = m_type != ReferenceType::None;
}

Reference::~Reference()
{
	delete m_simsExt;
}

Reference::Reference(const Tag* tag)
	: StanzaExtension(EXT_REFERENCES), m_valid(false),
	m_type(ReferenceType::None), m_uri(""), m_anchor(""), m_begin(-1),
	m_end(-1)
{
	// check that it's from the correct namespace
	if (!tag->hasAttribute(XMLNS, XMLNS_REFERENCES))
		return;

	// get type
	std::string type = tag->findAttribute("type");
	if (type == "mention")
		m_type = ReferenceType::Mention;
	else if (type == "data")
		m_type = ReferenceType::Data;

	// check for begin and end
	if (tag->hasAttribute("begin") && tag->hasAttribute("end")) {
		try {
			// conversion can cause invalid_argument / out_of_range exception
			m_begin = std::stoi(tag->findAttribute("start"));
			m_end = std::stoi(tag->findAttribute("end"));
		} catch (std::invalid_argument &e) {
			// Couldn't parse size: input probably doesn't contain valid number
		} catch (std::out_of_range &e) {
			// Couldn't parse size: is out of range of an unsigned long
		}
	}

	if (tag->hasAttribute("uri"))
		m_uri = tag->findAttribute("uri");
	if (tag->hasAttribute("anchor"))
		m_anchor = tag->findAttribute("anchor");

	// get embedded SIMS element
	Tag *simsTag = tag->findChild("media-sharing", XMLNS, XMLNS_SIMS);
	if (tag) {
		SIMS *sims = new SIMS(simsTag);
		if (sims) {
			m_simsExt = sims;
		}
	}

	m_valid = m_type != ReferenceType::None && m_uri.length();
}

Tag* Reference::tag() const
{
	Tag *tag = new Tag("reference", XMLNS, XMLNS_REFERENCES);
	if (!m_valid)
		return tag;

	// type
	if (m_type == ReferenceType::Mention)
		tag->addAttribute("type", "mention");
	else if (m_type == ReferenceType::Data)
		tag->addAttribute("type", "data");

	// uri and anchor
	if (!m_uri.empty())
		tag->addAttribute("uri", m_uri);
	if (!m_anchor.empty())
		tag->addAttribute("anchor", m_anchor);

	// begin and end
	if (m_begin >= 0 && m_end >= 0) {
		tag->addAttribute("begin", m_begin);
		tag->addAttribute("end", m_end);
	}

	// embedded stanza extension
	if (m_simsExt)
		tag->addChild(m_simsExt->tag());

	return tag;
}

const std::string& Reference::filterString() const
{
	static const std::string& filter = "/message/reference[@xmlns='" +
	                                   XMLNS_REFERENCES + "']";
	return filter;
}

void Reference::embedSIMS(SIMS* sims)
{
	m_simsExt = sims;
}
