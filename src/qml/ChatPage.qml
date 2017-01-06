/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
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
import QtQuick.Layouts 1.3
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.Page {
	property string chatName
	property string toJid

	title: chatName

	ColumnLayout {
		anchors.fill: parent

		//
		// Chat
		//

		ListView {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.margins: pane.leftPadding + messageField.leftPadding

			displayMarginBeginning: 40
			displayMarginEnd: 40

			verticalLayoutDirection: ListView.BottomToTop

			spacing: 12
			model: 10

			delegate: Row {
				readonly property bool sentByMe: index % 2 == 0

				anchors.right: sentByMe ? parent.right : undefined
				spacing: 6

				Rectangle {
					id: avatar

					width: height
					height: parent.height
					color: "grey"
					visible: !sentByMe
				}

				Rectangle {
					width: 80
					height: 40
					color: sentByMe ? "lightgrey" : "steelblue"

					Kirigami.Label {
						anchors.centerIn: parent
						text: index
						color: sentByMe ? "black" : "white"
					}
				}
			}

			//Controls.ScrollBar.vertical: Controls.ScrollBar {}
		}


		//
		// Message Writing
		//

		Controls.Pane {
			id: pane
			Layout.fillWidth: true

			RowLayout {
				width: parent.width

				Controls.TextArea {
					id: messageField
					Layout.fillWidth: true
					placeholderText: qsTr("Compose message")
					wrapMode: TextArea.Wrap
				}

				Controls.Button {
					id: sendButton
					text: qsTr("Send")
					enabled: messageField.length > 0
				}
			}
		}
	}
}
