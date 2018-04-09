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

#ifndef JINGLEFILE_H__
#define JINGLEFILE_H__

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include "hash.h"
#include "thumb.h"
#include <string>
#include <list>

namespace gloox {
	namespace Jingle {
		/**
		 * @class JingleFile An implementation of the file StanzaExtension of
		 * Jingle File Transfer (@xep{0234}).
		 *
		 * XEP Version: 0.18
		 *
		 * @author Linus Jahn <lnj@kaidan.im>
		 * @since 1.0.21
		 */
		class GLOOX_API File : public gloox::StanzaExtension
		{
		public:
			typedef std::list<Hash> HashList;

			/**
			 * Default constructor
			 *
			 * @param name The file's name.
			 * @param size The filesize in Bytes (use negative values to disable).
			 * @param hashes A list of Hashes.
			 * @param mediaType The file's MIME-type.
			 * @param date The file's last modification date
			 * @param desc A description.
			 * @param thumb A possible thumbnail for the content
			 * @param offset An offset for partial transfers (use negative values
			 * to disable).
			 * @param length The length from the offset used for a partial
			 * file transfer.
			 */
			File(const std::string& name = EmptyString, long size = -1,
			     HashList hashes = std::list<Hash>(),
			     const std::string& mediaType = EmptyString,
			     const std::string& date = EmptyString,
			     const std::string& desc = EmptyString,
			     const Thumb* thumb = nullptr,
			     long offset = -1, long length = -1);

			/**
			 * Construct from XML Tag
			 */
			File(const Tag *tag);

			/**
			 * Clone the StanzaExtension
			 */
			virtual StanzaExtension* clone() const
			{
				return new File(*this);
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
				return new File(tag);
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
			 * Get the file name
			 */
			std::string name() const
			{
				return m_name;
			}

			/**
			 * Get the file size
			 */
			long size() const
			{
				return m_size;
			}

			/**
			 * Get the media type (MIME-type)
			 */
			std::string mediaType() const
			{
				return m_mediaType;
			}

			/**
			 * Get the last modification date
			 */
			std::string date() const
			{
				return m_date;
			}

			/**
			 * Get the file description
			 */
			std::string desc() const
			{
				return m_desc;
			}

			/**
			 * Get the offset for a partial file transfer
			 */
			long rangeOffset() const
			{
				return m_offset;
			}

			/**
			 * Get the length from the offset for a partial file transfer
			 */
			long rangeLength() const
			{
				return m_length;
			}

			/**
			 * Get the list of hashes
			 */
			HashList hashes() const
			{
				return m_hashes;
			}

			/**
			 * Get a possible thumbnail/preview image
			 */
			const Thumb* thumb() const
			{
				return m_thumb;
			}

		private:
			bool m_valid;
			std::string m_name;
			long m_size;
			HashList m_hashes;
			std::string m_mediaType;
			std::string m_desc;
			std::string m_date;
			const Thumb* m_thumb;
			long m_offset;
			long m_length;
		};
	}
}

#endif // JINGLEFILE_H__
