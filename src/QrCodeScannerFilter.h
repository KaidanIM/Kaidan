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

#ifndef QRCODESCANNERFILTER_H
#define QRCODESCANNERFILTER_H

#include <QObject>
#include <QAbstractVideoFilter>
#include <QFuture>

#include <ZXing/DecodeHints.h>

#include "QrCodeDecoder.h"
#include "QrCodeVideoFrame.h"

/**
 * video filter to be registered in C++, instantiated and attached in QML
 */
class QrCodeScannerFilter : public QAbstractVideoFilter
{
	friend class QrCodeScannerFilterRunnable;

	Q_OBJECT
	Q_PROPERTY(QrCodeDecoder* decoder READ decoder)

public:
	/**
	 * Instantiates a QR code scanner filter.
	 *
	 * @param parent parent object
	 */
	explicit QrCodeScannerFilter(QObject *parent = nullptr);
	~QrCodeScannerFilter() override;

	/**
	 * @return decoder for decoding a video frame
	 */
	QrCodeDecoder *decoder();
	QVideoFilterRunnable *createFilterRunnable() override;

signals:
	/**
	 * Emitted when the scanning of an image did not succeed, i.e. no valid QR code was found.
	 */
	void scanningFailed();

	/**
	 * Emitted when the scanning of an image succeeded, i.e. a valid QR code was found and decoded.
	 *
	 * @param result decoded content of the QR code
	 */
	void scanningSucceeded(const QString& result);

	/**
	 * Emitted when a video frame with an unsupported format is received.
	 *
	 * @param format format of the video frame which is not supported
	 */
	void unsupportedFormatReceived(const QString& format);

private:
	QrCodeDecoder *m_decoder;

	/**
	 * frame of the video which may contain a QR code
	 */
	QrCodeVideoFrame m_frame;
	QFuture<void> m_processThread;
};

/**
 * runnable which is created everytime the filter gets a new frame
 */
class QrCodeScannerFilterRunnable : public QObject, public QVideoFilterRunnable
{
	Q_OBJECT

public:
	explicit QrCodeScannerFilterRunnable(QrCodeScannerFilter *m_filter);

	/**
	 * Runs the decoding in a new thread whenever a new frame is taken by the camera.
	 */
	QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags) override;

	/**
	 * Converts a given frame, which may contain a QR code, to an image and then tries to decode it.
	 *
	 * @param videoFrame frame to be converted and which may contain a QR code to be decoded
	 * @param filter filter of the current execution
	 */
	void processVideoFrameProbed(QrCodeVideoFrame videoFrame, QrCodeScannerFilter *filter);

private:
	QrCodeScannerFilter *m_filter;
};

#endif // QRCODESCANNERFILTER_H
