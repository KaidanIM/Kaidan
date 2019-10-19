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

#include "QrCodeScannerFilter.h"

#include <QDebug>
#include <QCamera>
#include <QCameraViewfinderSettings>
#include <QtConcurrent/QtConcurrent>

QrCodeScannerFilter::QrCodeScannerFilter(QObject *parent)
	: QAbstractVideoFilter(parent),
	  m_decoder(new QrCodeDecoder(this))
{
	connect(m_decoder, &QrCodeDecoder::decodingFailed,
			this, &QrCodeScannerFilter::scanningFailed);
	connect(m_decoder, &QrCodeDecoder::decodingSucceeded,
			this, &QrCodeScannerFilter::scanningSucceeded);
}

QrCodeScannerFilter::~QrCodeScannerFilter()
{
	if (!m_processThread.isFinished()) {
		m_processThread.cancel();
		m_processThread.waitForFinished();
	}
}

QrCodeDecoder *QrCodeScannerFilter::decoder()
{
	return m_decoder;
}

QVideoFilterRunnable *QrCodeScannerFilter::createFilterRunnable()
{
	return new QrCodeScannerFilterRunnable(this);
}

void QrCodeScannerFilter::setCameraDefaultVideoFormat(QObject *qmlCamera)
{
	QCamera *camera = qvariant_cast<QCamera*>(qmlCamera->property("mediaObject"));
	if (camera) {
		QCameraViewfinderSettings settings = camera->viewfinderSettings();
		settings.setPixelFormat(QVideoFrame::Format_RGB24);
		camera->setViewfinderSettings(settings);
	} else {
		qWarning() << "Could not set pixel format of QML camera";
	}
}

QrCodeScannerFilterRunnable::QrCodeScannerFilterRunnable(QrCodeScannerFilter *filter)
	: QObject(nullptr),
	m_filter(filter)
{
}

QVideoFrame QrCodeScannerFilterRunnable::run(
		QVideoFrame *input,
		const QVideoSurfaceFormat &,
		RunFlags
) {
	// Only one frame is processed at a time.
	if (input == nullptr
			|| !input->isValid()
			|| !m_filter->m_processThread.isFinished()) {
		return *input;
	}

	// Copy the data to be filtered.
	m_filter->m_frame.setData(*input);

	// Run a separate thread for processing the data.
	m_filter->m_processThread = QtConcurrent::run(
			this,
			&QrCodeScannerFilterRunnable::processVideoFrameProbed,
			m_filter->m_frame,
			m_filter
	);
	return *input;
}

void QrCodeScannerFilterRunnable::processVideoFrameProbed(
		QrCodeVideoFrame videoFrame,
		QrCodeScannerFilter *filter
) {
	// Return if the frame is empty.
	if (videoFrame.data().isEmpty())
		return;

	// Create an image from the frame.
	const QImage *image = videoFrame.toGrayscaleImage();

	// Return if conversion from the frame to the image failed.
	if (image->isNull()) {
		// dirty hack: write QVideoFrame::PixelFormat as string to format using QDebug
		//             QMetaEnum::valueToKey() did not work
		QString format;
		QDebug(&format).nospace() << videoFrame.pixelFormat();

		qDebug() << "QrCodeScannerFilterRunnable error: Cannot create image file to process.";
		qDebug() << "Maybe it was a format conversion problem.";
		qDebug() << "VideoFrame format:" << format;
		qDebug() << "Image corresponding format:"
		         << QVideoFrame::imageFormatFromPixelFormat(videoFrame.pixelFormat());

		emit filter->unsupportedFormatReceived(format);
		return;
	}

	// Decode the image.
	m_filter->decoder()->decodeImage(*image);

	delete image;
}
