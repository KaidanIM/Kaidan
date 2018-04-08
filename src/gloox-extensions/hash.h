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

#ifndef HASH_H__
#define HASH_H__

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include <string>

namespace gloox {
	/**
	 * @class Hash An implementation of 'Use of Cryptographic Hash Functions in
	 * XMPP' (@xep{0300}) as a StanzaExtension.
	 *
	 * XEP Version: 0.5
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API Hash : public gloox::StanzaExtension
	{
	public:
		/**
		 * Default constructor
		 *
		 * @param algo The hash algorithm used
		 * @param hash The hash binary value encoded as base64
		 */
		Hash(std::string algo, const std::string &hash = EmptyString);

		/**
		 * Construct from XML Tag
		 */
		Hash(const Tag *tag);

		/**
		 * Clone the StanzaExtension
		 */
		virtual StanzaExtension* clone() const
		{
			return new Hash(*this);
		}

		/**
		 * Get the XML tag
		 */
		virtual Tag* tag() const;

		/**
		 * Create a new instance
		 */
		virtual StanzaExtension* newInstance(const Tag* tag) const
		{
			return new Hash(tag);
		}

		/**
		 * Returns the filter string for filtering by the client
		 */
		virtual const std::string& filterString() const;

		/**
		 * Check if is valid
		 */
		bool valid() const
		{
			return m_valid;
		}

		/**
		 * Get hash algorithm
		 */
		std::string algorithm() const
		{
			return m_algo;
		}

		/**
		 * Get full hash value as base64 binary
		 */
		std::string value() const
		{
			return m_hash;
		}

	private:
		bool m_valid;
		std::string m_algo;
		std::string m_hash;
	};
}

#endif // PROCESSINGHINTS_H__
