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

#include "QXmppColorGenerator.h"
#include "../hsluv-c/hsluv.h"
#include <ctgmath>
#include <QCryptographicHash>

/// \brief Generates a color from the input value. This is intended for
/// generating colors for contacts. The generated colors are "consistent", so
/// they are shared between all clients with support for XEP-0392: Consistent
/// Color Generation.
///
/// \param name This should be the (user-specified) nickname of the
/// participant. If there is no nickname set, the bare JID shall be used.
/// \param deficiency Color correction to be done, defaults to
/// ColorVisionDeficiency::NoDeficiency.

QXmppColorGenerator::RGBColor QXmppColorGenerator::generateColor(
        const QString &name, ColorVisionDeficiency deficiency)
{
    QByteArray input = name.toUtf8();

    // hash input through SHA-1
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(input);

    // the first two bytes are used to calculate the angle/hue
    int angle = hash.result().at(0) + hash.result().at(1) * 256;

    double hue = double(angle) / 65536.0 * 360.0;
    double saturation = 100.0;
    double lightness = 50.0;

    // Corrections for Color Vision Deficiencies
    // this uses floating point modulo (fmod)
    if (deficiency == RedGreenBlindness) {
        hue += 90.0;
        hue = fmod(hue, 180);
        hue -= 90.0;
        hue = fmod(hue, 360);
    } else if (deficiency == BlueBlindness) {
        hue = fmod(hue, 180);
    }

    // convert to rgb values (values are between 0.0 and 1.0)
    double red, green, blue = 0.0;
    hsluv2rgb(hue, saturation, lightness, &red, &green, &blue);

    RGBColor color;
    color.red = quint8(red * 255.0);
    color.green = quint8(green * 255.0);
    color.blue = quint8(blue * 255.0);
    return color;
}
