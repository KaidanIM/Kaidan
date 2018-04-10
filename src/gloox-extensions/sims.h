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

#ifndef SIMS_H__
#define SIMS_H__

#include <gloox/stanzaextension.h>
#include <gloox/gloox.h>

namespace gloox {
	class Tag;
	namespace Jingle {
		class File;
	}

	/**
	 * @class SIMS An implementation of Stateless Inline Media Sharing (SIMS)
	 * (@xep{0372}) as a StanzaExtension.
	 *
	 * XEP Version: 0.2
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API SIMS : gloox::StanzaExtension
	{
	public:
		/**
		 * Default constructor
		 */
		SIMS(Jingle::File *file, StringList sources);

		/**
		 * Construct from XML Tag
		 */
		SIMS(const Tag *tag);

		/**
		 * Clone the current instance
		 */
		virtual StanzaExtension* clone() const
		{
			return new SIMS(*this);
		}

		/**
		 * Create a new instance
		 */
		virtual StanzaExtension* newInstance(const Tag* tag) const
		{
			return new SIMS(tag);
		}

		/**
		 * Create a new XML Tag of the StanzaExtension
		 *
		 * @return Empty tag, if not valid.
		 */
		virtual Tag* tag() const;

		/**
		 * Get a filter string for filtering in the client
		 */
		virtual const std::string& filterString() const;

		/**
		 * Returns true, if valid
		 */
		bool valid() const
		{
			return m_valid;
		}

		/**
		 * Get file information
		 */
		Jingle::File* file() const
		{
			return m_file;
		}

		/**
		 * Get list of source links
		 */
		StringList sources() const
		{
			return m_sources;
		}

	private:
		bool m_valid;
		Jingle::File *m_file;
		StringList m_sources;
	};
}

#endif // SIMS_H
