/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2017 JBBgameich <jbb.mail@gmx.de>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.0 as Kirigami

Row {
	id: root
	property bool sentByMe: true
	property string messageBody

	anchors.right: sentByMe ? parent.right : undefined
	spacing: Kirigami.Units.gridUnit * 0.5

	Rectangle {
		id: avatar

		width: Kirigami.Units.gridUnit * 2
		height: Kirigami.Units.gridUnit * 2
		radius: width * 0.5
		visible: !sentByMe
		color: "grey"
	}

	Item {
		height: label.implicitHeight + Kirigami.Units.gridUnit * 0.9
		width: label.width + Kirigami.Units.gridUnit

		Rectangle {
			id: box
			height: parent.height
			width: parent.width

			color: sentByMe ? "white" : "#4c9b4a"
			radius: 2
			border.width: 1
			border.color: "#E1DFDF"

			Kirigami.Label {
				id: label
				anchors.centerIn: parent
				width: Math.min(implicitWidth, pageStack.lastItem.width * 0.75)
				text: messageBody
				wrapMode: Text.Wrap
				color: sentByMe ? "black" : "white"
			}
		}
	}
}
