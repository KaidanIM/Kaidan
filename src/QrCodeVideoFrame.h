/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

/*
 * Copyright 2017 QZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef QRCODEVIDEOFRAME_H
#define QRCODEVIDEOFRAME_H

#include <QByteArray>
#include <QSize>
#include <QVideoFrame>
class QImage;

/**
 * video frame which may contain a QR code
 */
class QrCodeVideoFrame
{
public:
	/**
	 * Instantiates an empty video frame
	 */
	QrCodeVideoFrame()
	    : m_size{0,0},
	      m_pixelFormat{QVideoFrame::Format_Invalid}
	{}

	/**
	 * Sets the frame.
	 *
	 * @param frame frame to be set
	 */
	void setData(QVideoFrame &frame);

	/**
	 * Converts a given image to a grayscale image.
	 *
	 * @return grayscale image
	 */
	QImage *toGrayscaleImage();

	/**
	 * @return content of the frame which may contain a QR code
	 */
	QByteArray data() const;

	/**
	 * @return size of the frame
	 */
	QSize size() const;

	/**
	 * @return format of the frame
	 */
	QVideoFrame::PixelFormat pixelFormat() const;

private:
	QByteArray m_data;
	QSize m_size;
	QVideoFrame::PixelFormat m_pixelFormat;
};

#endif // QRCODEVIDEOFRAME_H
