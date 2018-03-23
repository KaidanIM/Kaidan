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

#ifndef HTTPUPLOADREQUEST_H__
#define HTTPUPLOADREQUEST_H__

#include <gloox/stanzaextension.h>
#include <gloox/gloox.h>

namespace gloox {
	class GLOOX_API HttpUploadRequest : public StanzaExtension
	{
	public:
		HttpUploadRequest();
		HttpUploadRequest(const std::string &filename, const unsigned long &size,
		                  const std::string &contentType = EmptyString);
		HttpUploadRequest(const gloox::Tag* tag);
		virtual ~HttpUploadRequest();
		
		virtual const std::string& filterString() const;
		virtual Tag* tag() const;

		virtual StanzaExtension* newInstance(const Tag *tag) const
		{
			return new HttpUploadRequest(tag);
		}

		virtual StanzaExtension* clone() const
		{
			return new HttpUploadRequest(*this);
		}

		std::string filename() const
		{
			return m_filename;
		}

		unsigned long size() const
		{
			return m_size;
		}

		std::string contentType() const
		{
			return m_contentType;
		}

		bool valid() const
		{
			return m_valid;
		}

	private:
		bool m_valid;
		std::string m_filename;
		unsigned long m_size;
		std::string m_contentType;
	};
}

#endif // HTTPUPLOADREQUEST_H__
