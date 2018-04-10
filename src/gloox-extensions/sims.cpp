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

#include "sims.h"
#include "gloox-extensions.h"
#include "jinglefile.h"
#include "reference.h"

using namespace gloox;

SIMS::SIMS(Jingle::File* file, StringList sources)
	: StanzaExtension(EXT_SIMS), m_file(file), m_sources(sources)
{
	m_valid = m_file->valid() && !m_sources.empty();
}

SIMS::SIMS(const Tag* tag)
	: StanzaExtension(EXT_SIMS), m_valid(false)
{
	if (tag->name() != "media-sharing" || tag->xmlns() != XMLNS_SIMS)
		return;

	if (tag->hasChild("file")) {
		Tag* fileTag = tag->findChild("file");
		Jingle::File* m_file = new Jingle::File(fileTag);
	}
	if (tag->hasChild("sources")) {
		Tag* sourcesTag = tag->findChild("sources");
		for (Tag* refTag : sourcesTag->findChildren("reference")) {
			if (refTag->xmlns() != XMLNS_REFERENCES)
				continue;

			Reference ref(refTag);
			if (ref.valid() && ref.type() == Reference::Data) {
				std::string sourceUri = ref.uri();
				if (sourceUri.length())
					m_sources.emplace_back(sourceUri);
			}
		}
	}

	m_valid = m_file && m_file->valid() && !m_sources.empty();
}

Tag* SIMS::tag() const
{
	Tag* tag = new Tag("media-sharing", XMLNS, XMLNS_SIMS);
	if (!m_valid || !m_file->valid() || m_sources.empty())
		return tag;

	// file
	tag->addChild(m_file->tag());

	// sources
	Tag* sources = new Tag("sources");
	for (std::string source : m_sources) {
		Reference sourceRef(Reference::Data, source);
		tag->addChild(sourceRef.tag());
	}

	return tag;
}

const std::string& SIMS::filterString() const
{
	static const std::string& filter = "/message/reference[@xmlns'" +
		XMLNS_REFERENCES + "']" + "/media-sharing[@xmlns='" + XMLNS_SIMS + "']";
	return filter;
}
