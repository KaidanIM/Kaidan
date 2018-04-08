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

#include "processinghints.h"
#include "gloox-extensions.h"

using namespace gloox;

ProcessingHints::ProcessingHints(ProcessingHints::Type type)
	: StanzaExtension(EXT_PROCESSINGHINTS), m_type(type), m_valid(type != None)
{
}

ProcessingHints::ProcessingHints(const Tag* tag)
	: StanzaExtension(EXT_PROCESSINGHINTS), m_type(None)
{
	std::map<std::string, ProcessingHints::Type> typeMap = {
		{"no-permanent-store", ProcessingHints::NoPermanentStore},
		{"no-store", ProcessingHints::NoStore},
		{"no-copy", ProcessingHints::NoCopies},
		{"store", ProcessingHints::Store}
	};

	if (typeMap.count(tag->name())) {
		m_type = typeMap[tag->name()];
	}

	m_valid = m_type != None;
}

Tag* ProcessingHints::tag() const
{
	std::map<std::string, ProcessingHints::Type> typeMap = {
		{"no-permanent-store", ProcessingHints::NoPermanentStore},
		{"no-store", ProcessingHints::NoStore},
		{"no-copy", ProcessingHints::NoCopies},
		{"store", ProcessingHints::Store}
	};

	for (std::pair<std::string, int> pair : typeMap) {
		if (m_type == pair.second) {
			return new Tag(pair.first, XMLNS, XMLNS_PROCESSINGHINTS);
		}
	}

	return nullptr;
}

const std::string& ProcessingHints::filterString() const
{
	static const std::string filter = "/message/*[@xmlns='" +
	                                  XMLNS_PROCESSINGHINTS + "']";
	return filter;
}
