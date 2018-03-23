/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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

#ifndef HTTPUPLOADSLOT_H__
#define HTTPUPLOADSLOT_H__

#include <gloox/stanzaextension.h>
#include <gloox/tag.h>
#include "gloox-extensions.h"

namespace gloox {
	class GLOOX_API HttpUploadSlot : public StanzaExtension
	{
	public:
		HttpUploadSlot();

		HttpUploadSlot(std::string &putUrl, std::string &getUrl,
		               std::map<std::string, std::string> &putHeader);

		HttpUploadSlot(const Tag *tag);

		virtual ~HttpUploadSlot();

		virtual const std::string& filterString() const;

		virtual Tag* tag() const;

		virtual StanzaExtension* newInstance(const Tag *tag) const
		{
			return new HttpUploadSlot(tag);
		}

		virtual StanzaExtension* clone() const
		{
			return new HttpUploadSlot(*this);
		}

		std::string putUrl() const
		{
			return m_putUrl;
		}

		std::string getUrl() const
		{
			return m_getUrl;
		}

		HeaderFieldMap putHeaderFields() const
		{
			return m_putHeader;
		}

		bool valid() const
		{
			return m_valid;
		}

	private:
		bool m_valid;
		std::string m_putUrl;
		std::string m_getUrl;
		std::map<std::string, std::string> m_putHeader;
	};
}

#endif // HTTPUPLOADSLOT_H__
