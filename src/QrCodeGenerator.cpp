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

#include "QrCodeGenerator.h"

#include <QDebug>
#include <QImage>
#include <QRgb>

#include "Kaidan.h"
#include "qxmpp-exts/QXmppUri.h"

#define COLOR_TABLE_INDEX_FOR_WHITE 0
#define COLOR_TABLE_INDEX_FOR_BLACK 1

QrCodeGenerator::QrCodeGenerator(QObject *parent)
	: QObject(parent)
{
}

QImage QrCodeGenerator::generateLoginUriQrCode(int edgePixelCount)
{
	QXmppUri uri;

	uri.setJid(Kaidan::instance()->getJid());
	uri.setAction(QXmppUri::Login);
	uri.setPassword(Kaidan::instance()->getPassword());

	return generateQrCode(uri.toString(), edgePixelCount);
}

QImage QrCodeGenerator::generateQrCode(const QString &text, int edgePixelCount)
{
	try {
		ZXing::MultiFormatWriter writer(ZXing::BarcodeFormat::QR_CODE);
		const ZXing::BitMatrix &bitMatrix = writer.encode(text.toStdWString(), edgePixelCount, edgePixelCount);
		return toImage(bitMatrix);
	} catch (const std::invalid_argument &e) {
		Kaidan::instance()->passiveNotificationRequested(tr("Generating the QR code failed: %1").arg(e.what()));
	}

	return {};
}

QImage QrCodeGenerator::toImage(const ZXing::BitMatrix &bitMatrix)
{
	QImage monochromeImage(bitMatrix.width(), bitMatrix.height(), QImage::Format_Mono);

	createColorTable(monochromeImage);

	for (int y = 0; y < bitMatrix.height(); ++y) {
		for (int x = 0; x < bitMatrix.width(); ++x) {
			int colorTableIndex = bitMatrix.get(x, y) ? COLOR_TABLE_INDEX_FOR_BLACK : COLOR_TABLE_INDEX_FOR_WHITE;
			monochromeImage.setPixel(y, x, colorTableIndex);
		}
	}

	return monochromeImage;
}

void QrCodeGenerator::createColorTable(QImage &blackAndWhiteImage)
{
	blackAndWhiteImage.setColor(COLOR_TABLE_INDEX_FOR_WHITE, qRgb(255, 255, 255));
	blackAndWhiteImage.setColor(COLOR_TABLE_INDEX_FOR_BLACK, qRgb(0, 0, 0));
}
