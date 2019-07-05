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

#ifndef QRCODEDECODER_H
#define QRCODEDECODER_H

#include <QObject>

/**
 * Decoder for QR codes. This is a backend for \c QrCodeScanner .
 */
class QrCodeDecoder : public QObject
{
	Q_OBJECT

public:
	/**
	 * Instantiates a QR code decoder.
	 *
	 * @param parent parent object
	 */
	explicit QrCodeDecoder(QObject *parent = nullptr);

signals:
	/**
	 * Emitted when the decoding failed.
	 */
	void decodingFailed();

	/**
	 * Emitted when the decoding succeeded.
	 *
	 * @param tag string which was decoded by the QR code decoder
	 */
	void decodingSucceeded(const QString &tag);

public slots:
	/**
	 * Tries to decode the QR code from the given image. When decoding has
	 * finished @c decodingFinished() will be emitted. In case a QR code was found,
	 * also @c tagFound() will be emitted.
	 *
	 * @param image image which may contain a QR code to decode to a string.
	 *        It needs to be in grayscale format (one byte per pixel).
	 */
	void decodeImage(const QImage &image);
};

#endif // QRCODEDECODER_H
