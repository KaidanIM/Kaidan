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

#include "QrCodeVideoFrame.h"
#include <QImage>
#include <cmath>

/**
 * rectangle of the video frame which may contain a QR code
 */
struct CaptureRect
{
	CaptureRect(const QRect& captureRect, int sourceWidth, int sourceHeight)
		: isValid(captureRect.x() >= 0 && captureRect.y() >= 0 && captureRect.isValid()),
		sourceWidth(sourceWidth),
		sourceHeight(sourceHeight),
		startX(isValid ? captureRect.x() : 0),
		targetWidth(isValid ? captureRect.width() : sourceWidth),
		endX(startX + targetWidth),
		startY(isValid ? captureRect.y() : 0),
		targetHeight(isValid ? captureRect.height() : sourceHeight),
		endY(startY + targetHeight)
	{}

	bool isValid;

	int sourceWidth;
	int sourceHeight;

	int startX;
	int targetWidth;
	int endX;

	int startY;
	int targetHeight;
	int endY;
};

uchar gray(uchar r, uchar g, uchar b)
{
	return (306 * (r & 0xFF) +
			601 * (g & 0xFF) +
			117 * (b & 0xFF) +
			0x200) >> 10;
}
uchar yuvToGray(uchar Y, uchar U, uchar V)
{
	const int C = int(Y) - 16;
	const int D = int(U) - 128;
	const int E = int(V) - 128;
	return gray(
		qBound<uchar>(0, uchar((298 * C + 409 * E + 128) >> 8), 255),
		qBound<uchar>(0, uchar((298 * C - 100 * D - 208 * E + 128) >> 8), 255),
		qBound<uchar>(0, uchar((298 * C + 516 * D + 128) >> 8), 255)
	);
}

uchar yuvToGray2(uchar y, uchar u, uchar v)
{
	double rD = y + 1.4075 * (v - 128);
	double gD = y - 0.3455 * (u - 128) - (0.7169 * (v - 128));
	double bD = y + 1.7790 * (u - 128);

	return gray(
		qBound<uchar>(0, uchar(floor(rD)), 255),
		qBound<uchar>(0, uchar(floor(gD)), 255),
		qBound<uchar>(0, uchar(floor(bD)), 255)
	);
}

static QImage* rgbDataToGrayscale(
		const uchar* data,
		const CaptureRect &captureRect,
		const int alpha,
		const int red,
		const int green,
		const int blue,
		const bool isPremultiplied = false
) {
	const int stride = (alpha < 0) ? 3 : 4;

	const int endX = captureRect.sourceWidth - captureRect.startX - captureRect.targetWidth;
	const int skipX = (endX + captureRect.startX) * stride;

	QImage *image_ptr = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
	uchar* pixelInit = image_ptr->bits();
	data += (captureRect.startY * captureRect.sourceWidth + captureRect.startX) * stride;

	for (int y = 1; y <= captureRect.targetHeight; ++y) {
		// Quick fix for iOS devices. Will be handled better in the future
#ifdef Q_OS_IOS
		uchar* pixel = pixelInit + (y - 1) * captureRect.targetWidth;
#else
		uchar* pixel = pixelInit + (captureRect.targetHeight - y) * captureRect.targetWidth;
#endif

		for (int x = 0; x < captureRect.targetWidth; ++x) {
			uchar r = data[red];
			uchar g = data[green];
			uchar b = data[blue];
			if (isPremultiplied) {
				uchar a = data[alpha];
				r = uchar((uint(r) * 255) / a);
				g = uchar((uint(g) * 255) / a);
				b = uchar((uint(b) * 255) / a);
			}
			*pixel = gray(r, g, b);
			++pixel;
			data += stride;
		}
		data += skipX;
	}

	return image_ptr;
}

void QrCodeVideoFrame::setData(QVideoFrame &frame)
{
	frame.map(QAbstractVideoBuffer::ReadOnly);

	// Copy video frame bytes to this.data.
	// This is made to try to get a better performance (less memory allocation, faster unmap)
	// Any other task is performed in a QFuture task, as we want to leave the UI thread asap.
	if (m_data.size() != frame.mappedBytes()) {
		m_data.resize(frame.mappedBytes());
	}
	memcpy(m_data.data(), frame.bits(), frame.mappedBytes());
	m_size = frame.size();
	m_pixelFormat = frame.pixelFormat();

	frame.unmap();
}

QImage *QrCodeVideoFrame::toGrayscaleImage()
{
	const CaptureRect captureRect(QRect(), m_size.width(), m_size.height());
	const auto* data = reinterpret_cast<const uchar *>(m_data.constData());
	const auto *yuvPtr = reinterpret_cast<const uint32_t *>(data);
	uchar *pixel;
	int wh;
	int w_2;
	int wh_54;

	QImage *image;
	switch (m_pixelFormat) {
	case QVideoFrame::Format_ARGB32:
		image = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3);
		break;
	case QVideoFrame::Format_ARGB32_Premultiplied:
		image = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3, true);
		break;
	case QVideoFrame::Format_RGB32:
		image = rgbDataToGrayscale(data, captureRect, 0, 1, 2, 3);
		break;
	case QVideoFrame::Format_RGB24:
		image = rgbDataToGrayscale(data, captureRect, -1, 0, 1, 2);
		break;
	// TODO: QVideoFrame::Format_RGB565
	// TODO: QVideoFrame::Format_RGB555
	// TODO: QVideoFrame::Format_ARGB8565_Premultiplied
	case QVideoFrame::Format_BGRA32:
		image = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0);
		break;
	case QVideoFrame::Format_BGRA32_Premultiplied:
		image = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0, true);
		break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
	case QVideoFrame::Format_ABGR32:
		image = rgbDataToGrayscale(data, captureRect, 0, 3, 2, 1);
		break;
#endif
	case QVideoFrame::Format_BGR32:
		image = rgbDataToGrayscale(data, captureRect, 3, 2, 1, 0);
		break;
	case QVideoFrame::Format_BGR24:
		image = rgbDataToGrayscale(data, captureRect, -1, 2, 1, 0);
		break;
	case QVideoFrame::Format_BGR565:
		/// This is a forced "conversion", colors end up swapped.
		image = new QImage(data, m_size.width(), m_size.height(), QImage::Format_RGB16);
		break;
	case QVideoFrame::Format_BGR555:
		/// This is a forced "conversion", colors end up swapped.
		image = new QImage(data, m_size.width(), m_size.height(), QImage::Format_RGB555);
		break;
	// TODO: QVideoFrame::Format_BGRA5658_Premultiplied
	case QVideoFrame::Format_YUV420P:
	case QVideoFrame::Format_NV12:
		/// nv12 format, encountered on macOS
		image = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
		pixel = image->bits();
		wh = m_size.width() * m_size.height();
		w_2 = m_size.width() / 2;
		wh_54 = wh * 5 / 4;

		for (int y = captureRect.startY; y < captureRect.endY; y++) {
			const int Y_offset = y * m_size.width();
			const int y_2 = y / 2;
			const int U_offset = y_2 * w_2 + wh;
			const int V_offset = y_2 * w_2 + wh_54;
			for (int x = captureRect.startX; x < captureRect.endX; x++) {
				const int x_2 = x / 2;
				const uchar Y = data[Y_offset + x];
				const uchar U = data[U_offset + x_2];
				const uchar V = data[V_offset + x_2];
				*pixel = yuvToGray(Y, U, V);
				++pixel;
			}
		}
		break;
	case QVideoFrame::Format_NV21:
		/// nv21 format, default on android
		/// image starts with a complete Y image, which we can use directly
		image = new QImage(data, captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
		break;
	case QVideoFrame::Format_YUYV:
		image = new QImage(captureRect.targetWidth, captureRect.targetHeight, QImage::Format_Grayscale8);
		pixel = image->bits();

		for (int y = captureRect.startY; y < captureRect.endY; y++){
			const uint32_t *row = &yuvPtr[y*(m_size.width()/2)];
			int end = captureRect.startX/2 + (captureRect.endX - captureRect.startX)/2;
			for (int x = captureRect.startX/2; x < end; x++){
				const uint8_t *pxl = reinterpret_cast<const uint8_t *>(&row[x]);
				const uint8_t y0 = pxl[0];
				const uint8_t u  = pxl[1];
				const uint8_t v  = pxl[3];
				const uint8_t y1 = pxl[2];

				*pixel = yuvToGray2(y0, u, v);
				++pixel;
				*pixel = yuvToGray2(y1, u, v);
				++pixel;
			}
		}

		break;
	// TODO: QVideoFrame::Format_IMC*
	// TODO: QVideoFrame::Format_*YUV*
	// TODO: QVideoFrame::Format_Y*
	// TODO: QVideoFrame::Format_Jpeg (needed?)
	default:
		image = new QImage(
			data,
			m_size.width(),
			m_size.height(),
			QVideoFrame::imageFormatFromPixelFormat(m_pixelFormat)
		);
		break;
	}
	return image;
}

QByteArray QrCodeVideoFrame::data() const
{
	return m_data;
}

QSize QrCodeVideoFrame::size() const
{
	return m_size;
}

QVideoFrame::PixelFormat QrCodeVideoFrame::pixelFormat() const
{
	return m_pixelFormat;
}
