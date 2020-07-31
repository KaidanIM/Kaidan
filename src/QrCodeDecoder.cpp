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

#include "QrCodeDecoder.h"
// Qt
#include <QColor>
#include <QImage>
// ZXing-cpp
#include <ZXing/BarcodeFormat.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/GenericLuminanceSource.h>
#include <ZXing/HybridBinarizer.h>
#include <ZXing/LuminanceSource.h>
#include <ZXing/MultiFormatReader.h>
#include <ZXing/Result.h>
#include <ZXing/TextUtfEncoding.h>
#include <ZXing/ZXVersion.h>

#define ZXING_VERSION \
	QT_VERSION_CHECK(ZXING_VERSION_MAJOR, ZXING_VERSION_MINOR, ZXING_VERSION_PATCH)

using namespace ZXing;

QrCodeDecoder::QrCodeDecoder(QObject *parent)
	: QObject(parent)
{
}

void QrCodeDecoder::decodeImage(const QImage &image)
{
	// Advise the decoder to check for QR codes and to try decoding rotated versions of the image.
#if ZXING_VERSION >= QT_VERSION_CHECK(1, 1, 0)
	const auto decodeHints = DecodeHints().setTryRotate(true).setFormats(BarcodeFormat::QR_CODE);
#else
	const auto decodeHints =
		DecodeHints().setTryRotate(true).setPossibleFormats({BarcodeFormat::QR_CODE});
#endif

	// Create a binarized image by the luminance of the image.
	HybridBinarizer binImage(std::make_shared<GenericLuminanceSource>(GenericLuminanceSource(
		image.width(), image.height(), image.bits(), image.width(), 1, 0, 1, 2)));

	// Decode the specific image source.
	auto result = MultiFormatReader(decodeHints).read(binImage);

	// If a QR code could be found and decoded, emit a signal with the decoded string.
	// Otherwise, emit a signal for failed decoding.
	if (result.isValid())
		emit decodingSucceeded(QString::fromStdString(TextUtfEncoding::ToUtf8(result.text())));
	else
		emit decodingFailed();
}
