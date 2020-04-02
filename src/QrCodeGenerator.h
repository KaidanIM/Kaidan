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

#pragma once

#include <QObject>

#include <ZXing/BitMatrix.h>

class QImage;

class QrCodeGenerator : public QObject
{
	Q_OBJECT

public:
	/**
	 * Instantiates a QR code generator.
	 *
	 * @param parent parent object
	 */
	explicit QrCodeGenerator(QObject *parent = nullptr);

	/**
	 * Gerenates a QR code encoding the credentials of the currently used account to log into it with another client.
	 *
	 * @param edgePixelCount number of pixels as the width and height of the QR code
	 */
	Q_INVOKABLE static QImage generateLoginUriQrCode(int edgePixelCount);

	/**
	 * Gerenates a QR code encoding an XMPP URI containing a given bare JID.
	 *
	 * @param edgePixelCount number of pixels as the width and height of the QR code
	 * @param bareJid bare JID being encoded in an XMPP URI
	 */
	Q_INVOKABLE static QImage generateBareJidQrCode(int edgePixelCount, const QString &bareJid);

	/**
	 * Gerenates a QR code.
	 *
	 * @param edgePixelCount number of pixels as the width and height of the QR code
	 * @param text string being encoded as a QR code
	 */
	Q_INVOKABLE static QImage generateQrCode(int edgePixelCount, const QString &text);

private:
	/**
	 * Generates an image with black and white pixels from a given matrix of bits representing a QR code.
	 *
	 * @param bitMatrix matrix of bits representing the two colors black and white
	 */
	static QImage toImage(const ZXing::BitMatrix &bitMatrix);

	/**
	 * Sets up a color image for a given monochrome image consisting only of black and white pixels.
	 * @param blackAndWhiteImage image for which a color table with the colors black and white is created
	 */
	static void createColorTable(QImage &blackAndWhiteImage);
};
