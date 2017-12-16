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

import QtQuick 2.6
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami

Row {
	id: root
	property bool sentByMe: true
	property string messageBody: ""
	property date dateTime: new Date()
	property bool isRead: false
	property string recipientAvatarUrl: ""

	anchors.right: sentByMe ? parent.right : undefined
	spacing: Kirigami.Units.gridUnit * 0.5
	layoutDirection: sentByMe ? Qt.RightToLeft : Qt.LeftToRight

	RoundImage {
		id: avatar
		visible: !sentByMe
		width: Kirigami.Units.gridUnit * 2.2
		height: Kirigami.Units.gridUnit * 2.2
		source: recipientAvatarUrl
	}

	Item {
		id: bubble
		height: label.implicitHeight + messageInfo.height
		width: Math.max(label.width, checkmark.visible ? dateLabel.width + checkmark.width + Kirigami.Units.gridUnit * 1.7
							       : dateLabel.width + checkmark.width + Kirigami.Units.gridUnit * 0.7)

		Rectangle {
			id: box
			height: parent.height
			width: parent.width
			color: sentByMe ? "white" : "#4c9b4a"
			radius: 2
			border.width: 1
			border.color: "#E1DFDF"
		}

		Column {
			id: layout

			Controls.Label {
				id: label
				width: Math.min(implicitWidth, pageStack.lastItem.width * 0.8)
				height: implicitHeight
				leftPadding: Kirigami.Units.gridUnit * 0.5
				rightPadding: Kirigami.Units.gridUnit * 0.5
				topPadding: Kirigami.Units.gridUnit * 0.5
				bottomPadding: Kirigami.Units.gridUnit * 0.2
				text: messageBody
				wrapMode: Text.Wrap
				color: sentByMe ? "black" : "white"
			}

			Row {
				id: messageInfo
				leftPadding: Kirigami.Units.gridUnit * 0.5
				rightPadding: Kirigami.Units.gridUnit * 0.5
				bottomPadding: Kirigami.Units.gridUnit * 0.5
				spacing: Kirigami.Units.gridUnit * 0.5

				Controls.Label {
					id: dateLabel
					height: implicitHeight
                                        text: Qt.formatDateTime(dateTime, "dd MMM, hh:mm")
					color: sentByMe ? "grey" : "#e0e0e0"
                                }
				
				Image {
					id: checkmark
					visible: (sentByMe && isRead)
					anchors.verticalCenter: dateLabel.verticalCenter
					height: Kirigami.Units.gridUnit * 0.6
					width: Kirigami.Units.gridUnit * 0.6
					source: kaidan.getResourcePath("images/message_checkmark.svg");
					mipmap: true
				}
			}
		}
	}
}
