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

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtGraphicalEffects 1.12
import org.kde.kirigami 2.12 as Kirigami

ChatPageBase {
	verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

	Item {
		height: root.height * 0.8

		// message bubble
		Kirigami.ShadowedRectangle {
			anchors.centerIn: parent
			width: label.width
			height: label.height
			shadow.color: Qt.darker(color, 1.2)
			shadow.size: 4
			radius: roundedCornersRadius
			color: rightMessageBubbleColor

			Controls.Label {
				id: label
				text: qsTr("Please select a chat to start messaging")
				anchors.centerIn: parent
				padding: Kirigami.Units.gridUnit * 0.4
			}
		}
	}
}
