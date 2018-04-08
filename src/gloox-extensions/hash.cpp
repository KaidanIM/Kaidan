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

#include "hash.h"
#include "gloox-extensions.h"

using namespace gloox;

Hash::Hash(std::string algo, const std::string& hash)
	: StanzaExtension(EXT_HASHES), m_algo(hash), m_hash(hash),
	m_valid(m_algo.length())
{
}

Hash::Hash(const Tag* tag)
	: StanzaExtension(EXT_HASHES), m_valid(false)
{
	if (tag->findAttribute("xmlns") != XMLNS_HASHES || !tag->hasAttribute("algo"))
		return;

	m_algo = tag->findAttribute("algo");

	if (tag->name() == "hash") {
		m_hash = tag->cdata();
		m_valid = m_algo.length() && m_hash.length();
	} else if (tag->name() == "hash-used") {
		m_valid = m_algo.length();
	}
}

Tag* Hash::tag() const
{
	if (!m_valid)
		return new Tag("hash");

	if (m_hash.length()) {
		Tag* tag = new Tag("hash", XMLNS, XMLNS_HASHES);
		tag->addAttribute("algo", m_algo);
		tag->addCData(m_hash);
		return tag;
	}

	Tag* tag = new Tag("hash-used", XMLNS, XMLNS_HASHES);
	tag->addAttribute("algo", m_algo);
	return tag;
}

const std::string& Hash::filterString() const
{
	static const std::string filter = "/*/*[@xmlns='" + XMLNS_HASHES + "']";
	return filter;
}
