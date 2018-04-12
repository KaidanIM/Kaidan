/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami
import QtQuick 2.7

Kirigami.Page {
	background: Image {
		id: bgimage
		source: kaidan.getResourcePath("images/chat.png")
		anchors.fill: parent
		fillMode: Image.Tile
		horizontalAlignment: Image.AlignLeft
		verticalAlignment: Image.AlignTop
	}

	Rectangle {
		height: text.implicitHeight
		width: text.implicitWidth

		color: "lightgrey"
		anchors.centerIn: parent
		radius: Kirigami.Units.gridUnit

		Controls.Label {
			id: text
			anchors.centerIn: parent

			leftPadding: Kirigami.Units.gridUnit * 0.4
			rightPadding: Kirigami.Units.gridUnit * 0.4
			topPadding: Kirigami.Units.gridUnit * 0.4
			bottomPadding: Kirigami.Units.gridUnit * 0.4

			text: qsTr("Please select a contact to start messaging")
		}
	}
}
