/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

#ifndef BITSOFBINARYIMAGEPROVIDER_H
#define BITSOFBINARYIMAGEPROVIDER_H

// Qt
#include <QQuickImageProvider>
#include <QMutex>
#include <QVector>
// QXmpp
#include <QXmppBitsOfBinaryData.h>

/**
 * Provider for images received via XEP-0231: Bits of Binary
 *
 * @note This class is thread-safe.
 */
class BitsOfBinaryImageProvider : public QQuickImageProvider
{
public:
	static BitsOfBinaryImageProvider *instance();

	BitsOfBinaryImageProvider();
	~BitsOfBinaryImageProvider();

	/**
	 * Creates a QImage from the cached data.
	 *
	 * @param id BitsOfBinary content URL of the requested image (starting with "cid:").
	 * @param size size of the cached image.
	 * @param requestedSize size the image should be scaled to. If this is invalid the image
	 * is not scaled.
	 */
	QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

	/**
	 * Adds @c QXmppBitsOfBinaryData to the cache used to provide images to QML.
	 *
	 * @param data Image in form of a Bits of Binary data element.
	 *
	 * @return True if the data has a supported image format. False if it could not be
	 * added to the cache.
	 */
	bool addImage(const QXmppBitsOfBinaryData &data);

	/**
	 * Removes the BitsOfBinary data associated with the given content ID from the cache.
	 *
	 * @param cid BitsOfBinary content ID to search for.
	 *
	 * @return True if the content ID could be found and the image was removed, False
	 * otherwise.
	 */
	bool removeImage(const QXmppBitsOfBinaryContentId &cid);

private:
	static BitsOfBinaryImageProvider *s_instance;
	QMutex m_cacheMutex;
	QVector<QXmppBitsOfBinaryData> m_cache;
};

#endif // BITSOFBINARYIMAGEPROVIDER_H
