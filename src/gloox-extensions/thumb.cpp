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

#include "thumb.h"
#include "gloox-extensions.h"
#include <QString>

using namespace gloox;
using namespace gloox::Jingle;

Thumb::Thumb(std::string uri, int width, int height, std::string mediaType)
	: StanzaExtension(EXT_THUMBS), m_uri(uri), m_width(width), m_height(height),
	m_mediaType(mediaType), m_valid(!uri.empty() && width && height)
{
}

Thumb::Thumb(const Tag* tag)
	: StanzaExtension(EXT_THUMBS), m_valid(false)
{
	if (!tag || tag->findAttribute("xmlns") != XMLNS_THUMBS ||
	    !tag->hasAttribute("uri") || !tag->hasAttribute("width") ||
	    !tag->hasAttribute("height"))
		return;

	m_uri = tag->findAttribute("uri");
	if (tag->hasAttribute("media-type"))
		m_mediaType = tag->findAttribute("media-type");
	try {
		// conversion can cause invalid_argument / out_of_range exception
		m_width = QString::fromStdString(tag->findAttribute("width")).toULong();
		m_height = QString::fromStdString(tag->findAttribute("height")).toULong();
	} catch (std::invalid_argument &e) {
		// Couldn't parse size: input probably doesn't contain valid number
	} catch (std::out_of_range &e) {
		// Couldn't parse size: is out of range of an unsigned long
	}

	m_valid = !m_uri.empty() && m_width && m_height;
}

Tag* Thumb::tag() const
{
	if (!m_valid)
		return new Tag("thumb");

	Tag* tag = new Tag("thumb", XMLNS, XMLNS_THUMBS);
	tag->addAttribute("uri", m_uri);
	if (!m_mediaType.empty())
		tag->addAttribute("media-type", m_mediaType);
	tag->addAttribute("width", m_width);
	tag->addAttribute("height", m_height);

	return tag;
}

const std::string& Thumb::filterString() const
{
	static const std::string filter = "thumb[@xmlns='" + XMLNS_THUMBS + "']";
	return filter;
}
