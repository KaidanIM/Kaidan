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

#ifndef THUMB_H__
#define THUMB_H__

#include <gloox/gloox.h>
#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include <string>

namespace gloox {
	namespace Jingle {
		/**
		 * @class Thumb An implementation of Jingle Content Thumbnails
		 * (@xep{0264}) as a StanzaExtension.
		 *
		 * XEP Version: 0.4
		 *
		 * @author Linus Jahn <lnj@kaidan.im>
		 * @since 1.0.21
		 */
		class GLOOX_API Thumb : public gloox::StanzaExtension
		{
		public:
			/**
			 * Default constructor
			 *
			 * @param uri The @c BitsOfBinaryData identifier (but starting
			 * with 'cid:')
			 * @param width Width of the thumbnail image
			 * @param height Height of the thumbnail image
			 * @param mediaType MIME-type of the image
			 */
			Thumb(std::string uri, int width, int height,
			      std::string mediaType = "");

			/**
			 * Construct from XML Tag
			 */
			Thumb(const Tag *tag);

			/**
			 * Clone the StanzaExtension
			 */
			virtual StanzaExtension* clone() const
			{
				return new Thumb(*this);
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
				return new Thumb(tag);
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

			std::string uri() const
			{
				return m_uri;
			}

			int width() const
			{
				return m_width;
			}

			int height() const
			{
				return m_height;
			}

			std::string mediaType() const
			{
				return m_mediaType;
			}

		private:
			bool m_valid;
			std::string m_uri;
			std::string m_mediaType;
			int m_width;
			int m_height;
		};
	}
}

#endif // THUMB_H__
