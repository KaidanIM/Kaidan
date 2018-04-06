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

#ifndef REFERENCE_H__
#define REFERENCE_H__

#include <gloox/stanzaextension.h>
#include <gloox/gloox.h>

namespace gloox {
	class Tag;
	class SIMS;

	/**
	 * @class Reference An implementation of References (@xep{0372})
	 * as a StanzaExtension.
	 *
	 * It can be used to provide references to another entity, such as
	 * mentioning users, HTTP resources, or other XMPP resources.
	 *
	 * If you support them, you need to include them into your service
	 * discovery features:
	 *
	 * @code
	 * client->disco()->addFeature(XMLNS_REFERENCES);
	 * @endcode
	 *
	 * XEP Version: 0.2
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API Reference : public gloox::StanzaExtension
	{
	public:
		enum ReferenceType {
			None,
			Mention,
			Data
		};

		/**
		 * Default constructor
		 */
		Reference(ReferenceType type, int begin = -1, int end = -1,
		          std::string uri = "", std::string anchor = "");

		/**
		 * Simple constructor
		 */
		Reference(ReferenceType type, std::string& uri);

		/**
		 * Construct from XML Tag
		 */
		Reference(const Tag *tag);

		/**
		 * Default destructor
		 */
		~Reference();

		/**
		 * Clone the current instance
		 */
		virtual StanzaExtension* clone() const
		{
			return new Reference(*this);
		}

		/**
		 * Create a new instance
		 */
		virtual StanzaExtension* newInstance(const Tag* tag) const
		{
			return new Reference(tag);
		}

		/**
		 * Create a new XML Tag from of the Stanza
		 */
		virtual Tag* tag() const;

		/**
		 * Get a filter string for filtering in the client
		 */
		virtual const std::string& filterString() const;

		/**
		 * Add a @c SIMS element to embed it into the reference
		 * This will override possible SIMS elements added before.
		 */
		void embedSIMS(SIMS *sims);

		/**
		 * Returns true if stanza extension is valid
		 */
		bool valid() const
		{
			return m_valid;
		}

		/**
		 * Get type of reference
		 */
		ReferenceType type() const
		{
			return m_type;
		}

		/**
		 * Get URI
		 */
		std::string uri() const
		{
			return m_uri;
		}

		/**
		 * Get anchor
		 */
		std::string anchor() const
		{
			return m_anchor;
		}

		/**
		 * Get begin (negative value, if not set)
		 */
		int begin() const
		{
			return m_begin;
		}

		/**
		 * Get end (negative value, if not set)
		 */
		int end() const
		{
			return m_end;
		}

		/**
		 * Get embedded @c SIMS element
		 *
		 * May be a nullptr
		 */
		SIMS* getEmbeddedSIMS() const
		{
			return m_simsExt;
		}

	private:
		bool m_valid;
		ReferenceType m_type = None;
		std::string m_uri;
		std::string m_anchor;
		int m_begin;
		int m_end;
		SIMS *m_simsExt;
	};
}

#endif // REFERENCE_H
