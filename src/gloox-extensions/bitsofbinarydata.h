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

#ifndef BITSOFBINARYDATA_H__
#define BITSOFBINARYDATA_H__

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include <string>

namespace gloox {
	/**
	 * @class BitsOfBinaryData An implementation of Bits of Binary (@xep{0231})
	 * as a StanzaExtension.
	 *
	 * XEP Version: 1.0
	 *
	 * @author Linus Jahn <lnj@kaidan.im>
	 * @since 1.0.21
	 */
	class GLOOX_API BitsOfBinaryData : public gloox::StanzaExtension
	{
	public:
		/**
		 * Create an empty Bits of Binary data StanzaExtension, this is needed to
		 * request data from another entity.
		 *
		 * @param cid The content ID
		 */
		BitsOfBinaryData(const std::string cid);

		/**
		 * Create a full Bits of Binary data StanzaExtension. This is needed to
		 * reply to a request and send the asked data.
		 *
		 * @param cid The content ID (will be generated if empty string)
		 * @param data The base64 encoded data (should not be larger than 8 KiB)
		 * @param contentType The mime type of the content
		 * @param maxAge Time in seconds the data should be cached, after that
		 * time limit the receiving entity should delete the data. 0 means to not
		 * cache it at all. Use negative values for no limit.
		 */
		BitsOfBinaryData(std::string &data, std::string cid,
		                 std::string contentType = "", long maxAge = -1);

		/**
		 * Construct from XML Tag
		 */
		BitsOfBinaryData(const Tag *tag);

		/**
		 * Clone the StanzaExtension
		 */
		virtual StanzaExtension* clone() const
		{
			return new BitsOfBinaryData(*this);
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
			return new BitsOfBinaryData(tag);
		}

		/**
		 * Returns the filter string for filtering by the client
		 */
		virtual const std::string& filterString() const;

		/**
		 * Returns if the StanzaExtension is valid
		 */
		bool valid() const
		{
			return m_valid;
		}

		/**
		 * Returns the content ID
		 */
		const std::string& contentId() const
		{
			return m_cid;
		}

		/**
		 * Returns the time in seconds the content should be cached. 0 for no
		 * caching, -1 for no limitation.
		 */
		long maxAge() const
		{
			return m_maxAge;
		}

		/**
		 * Returns the content type of the data (can be empty).
		 */
		std::string contentType() const
		{
			return m_contentType;
		}

		/**
		 * Returns the base64 encoded data.
		 */
		const std::string& data() const
		{
			return m_data;
		}

		/**
		 * Generates a new Content ID (cid)
		 *
		 * @param data The data of the content
		 * @param isBase64 if the data is Base64 encoded (true) or the data is
		 * already in binary format (false)
		 */
		static const std::string generateContentId(const std::string &data,
		                                           bool isBase64 = true);

	private:
		bool m_valid;
		std::string m_cid;
		long m_maxAge;
		std::string m_data;
		std::string m_contentType;
	};
}

#endif // BITSOFBINARYDATA_H__
