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

#include "bitsofbinarydata.h"
#include "gloox-extensions.h"
#include <gloox/gloox.h>
#include <gloox/base64.h>
#include <gloox/sha.h>
#include <QString>

using namespace gloox;

BitsOfBinaryData::BitsOfBinaryData(std::string cid)
	: StanzaExtension(EXT_BITSOFBINARY), m_cid(cid),
	m_data(""), m_contentType(""), m_maxAge(-1),
	m_valid(m_cid.length() >= 50)
{
}

BitsOfBinaryData::BitsOfBinaryData(std::string& data, std::string cid,
                                   std::string contentType, long maxAge)
	: StanzaExtension(EXT_BITSOFBINARY), m_cid(cid), m_data(data),
	m_contentType(contentType), m_maxAge(maxAge)
{
	if (!m_cid.length())
		m_cid = generateContentId(m_data);

	m_valid = m_cid.length() >= 58 && m_data.length();
}

BitsOfBinaryData::BitsOfBinaryData(const Tag* tag)
	: StanzaExtension(EXT_BITSOFBINARY), m_valid(false), m_cid(""),
	m_maxAge(-1), m_data(""), m_contentType("")
{
	if (tag->name() != "data" || !tag->hasAttribute(XMLNS, XMLNS_BITSOFBINARY)) {
		return;
	}

	m_cid = tag->findAttribute("cid");
	m_contentType = tag->findAttribute("type");
	m_data = tag->cdata();

	if (tag->hasAttribute("max-age")) {
		try {
			// conversion can cause invalid_argument / out_of_range exception
			m_maxAge = QString::fromStdString(tag->findAttribute("max-age")).toLong();
		} catch (std::invalid_argument &e) {
			// Couldn't parse size: input probably doesn't contain valid number
		} catch (std::out_of_range &e) {
			// Couldn't parse size: is out of range of an long
		}
	}

	m_valid = true;
}

Tag* BitsOfBinaryData::tag() const
{
	Tag* bobTag = new Tag("data", XMLNS, XMLNS_BITSOFBINARY);
	// if not valid, return empty slot tag
	if (!m_valid)
		return bobTag;

	bobTag->addAttribute("cid", m_cid);
	if (m_maxAge >= 0)
		bobTag->addAttribute("max-age", m_maxAge);
	if (m_contentType.length() > 0)
		bobTag->addAttribute("type", m_contentType);
	if (m_data.length() > 0)
		bobTag->addCData(m_data);

	return bobTag;
}

const std::string& BitsOfBinaryData::filterString() const
{
	static const std::string filter = "/iq/data[@xmlns='" + XMLNS_BITSOFBINARY +
	                                  "']";
	return filter;
}

const std::string BitsOfBinaryData::generateContentId(const std::string& data,
                                                      bool isBase64)
{
	std::string sha1Hex;
	SHA hashGenerator;

	if (isBase64)
		hashGenerator.feed(Base64::decode64(data));
	else
		hashGenerator.feed(data);

	hashGenerator.finalize();
	sha1Hex = hashGenerator.hex();
	return "sha1+" + sha1Hex + "@bob.xmpp.org";
}
