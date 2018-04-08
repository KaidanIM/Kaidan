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

#ifndef PROCESSINGHINTS_H__
#define PROCESSINGHINTS_H__

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include <string>

namespace gloox {
	/**
	 * @class ProcessingHints An implementation of Message Processing Hints
	 * (@xep{0334}) as a StanzaExtension.
	 *
	 * XEP Version: 0.3
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API ProcessingHints : public gloox::StanzaExtension
	{
	public:
		enum Type {
			None,             /* No Message Processing Hint */
			NoPermanentStore, /* Entities shouldn't store permanently */
			NoStore,          /* Entities shouldn't store at all */
			NoCopies,         /* Messages shouldn't be copied to other than given entity */
			Store             /* Message should be explicitly stored */
		};

		/**
		 * Default constructor
		 *
		 * @param type The type of processing hint
		 */
		ProcessingHints(Type type);

		/**
		 * Construct from XML Tag
		 */
		ProcessingHints(const Tag *tag);

		/**
		 * Clone the StanzaExtension
		 */
		virtual StanzaExtension* clone() const
		{
			return new ProcessingHints(*this);
		}

		/**
		 * Get the XML tag
		 * 
		 * @warning This will return a nullptr, if the object is not valid.
		 */
		virtual Tag* tag() const;

		/**
		 * Create a new instance
		 */
		virtual StanzaExtension* newInstance(const Tag* tag) const
		{
			return new ProcessingHints(tag);
		}

		/**
		 * Returns the filter string for filtering by the client
		 */
		virtual const std::string& filterString() const;

		/**
		 * Get the type
		 */
		ProcessingHints::Type type() const
		{
			return m_type;
		}

		/**
		 * Check if is valid
		 */
		bool valid() const
		{
			return m_valid;
		}

	private:
		bool m_valid;
		Type m_type;
	};
}

#endif // PROCESSINGHINTS_H__
