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

#include "BitsOfBinaryImageProvider.h"

// Qt
#include <QImageReader>
#include <QMimeType>
#include <QMutexLocker>
// QXmpp
#include <QXmppBitsOfBinaryContentId.h>

BitsOfBinaryImageProvider *BitsOfBinaryImageProvider::s_instance;

BitsOfBinaryImageProvider *BitsOfBinaryImageProvider::instance()
{
	if (s_instance == nullptr)
		s_instance = new BitsOfBinaryImageProvider();

	return s_instance;
}

BitsOfBinaryImageProvider::BitsOfBinaryImageProvider()
	: QQuickImageProvider(QQuickImageProvider::Image)
{
	Q_ASSERT(!s_instance);
	s_instance = this;
}

BitsOfBinaryImageProvider::~BitsOfBinaryImageProvider()
{
	s_instance = nullptr;
}

QImage BitsOfBinaryImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
	QMutexLocker locker(&m_cacheMutex);

	const auto item = std::find_if(m_cache.begin(), m_cache.end(), [=] (const QXmppBitsOfBinaryData &item) {
		return item.cid().toCidUrl() == id;
	});

	if (item == m_cache.end())
		return {};

	QImage image = QImage::fromData((*item).data());
	size->setWidth(image.width());
	size->setHeight(image.height());

	if (requestedSize.isValid())
		image = image.scaled(requestedSize);

	return image;
}

bool BitsOfBinaryImageProvider::addImage(const QXmppBitsOfBinaryData &data)
{
	QMutexLocker locker(&m_cacheMutex);

	if (!QImageReader::supportedMimeTypes().contains(data.contentType().name().toUtf8())) {
		return false;
	}

	m_cache << data;
	return true;
}

bool BitsOfBinaryImageProvider::removeImage(const QXmppBitsOfBinaryContentId &cid)
{
	QMutexLocker locker(&m_cacheMutex);

	return m_cache.end() != std::remove_if(m_cache.begin(), m_cache.end(), [&] (const QXmppBitsOfBinaryData &item) {
		return item.cid() == cid;
	});
}
