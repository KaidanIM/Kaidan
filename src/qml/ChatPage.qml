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
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami

Kirigami.Page {
	id: root

	property string chatName
	property string recipientJid

	title: chatName

	leftPadding: 0
	rightPadding: 0
	bottomPadding: 0
	topPadding: 0

	Image {
		id: bgimage
		source: kaidan.getResourcePath("images/chat.png")
		anchors.fill: parent
		fillMode: Image.Tile
		horizontalAlignment: Image.AlignLeft
		verticalAlignment: Image.AlignTop
	}

	ColumnLayout {
		anchors.fill: parent

		//
		// Chat
		//

		ListView {
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.margins: Kirigami.Units.gridUnit * 0.75

			displayMarginBeginning: 40
			displayMarginEnd: 40

			verticalLayoutDirection: ListView.BottomToTop

			spacing: 12

			// connect the database
			model: kaidan.messageModel

			delegate: Row {
				readonly property bool sentByMe: model.recipient !== kaidan.jid

				anchors.right: sentByMe ? parent.right : undefined
				spacing: 6

				Rectangle {
					id: avatar

					width: 40
					height: 40
					radius: width * 0.5

					color: "grey"
					visible: !sentByMe
				}

				Rectangle {
					width: messageText.width + 12
					height: messageText.height + 6

					radius: 2
					border.width: 1
					border.color: "#E1DFDF"

					color: sentByMe ? "lightgrey" : "steelblue"

					Kirigami.Label {
						id: messageText

						anchors.centerIn: parent

						text: model.message
						wrapMode: Text.Wrap
						color: sentByMe ? "black" : "white"
					}

					Component.onCompleted: {
						if (messageText.paintedWidth > 190) {
							messageText.width = 190
						}
					}
				}
			}
		}


		//
		// Message Writing
		//

		Controls.Pane {
			id: sendingArea
			Layout.fillWidth: true
			topPadding: Kirigami.Units.gridUnit * 0.1
			bottomPadding: Kirigami.Units.gridUnit * 0.1
			wheelEnabled: true
			background: Rectangle {
				color: "white"
			}

			RowLayout {
				width: parent.width

				Controls.TextArea {
					id: messageField
					Layout.fillWidth: true
					placeholderText: qsTr("Compose message")
					wrapMode: Controls.TextArea.Wrap
					selectByMouse: true
				}

				Controls.Button {
					id: sendButton
					text: qsTr("Send")
					onClicked: {
						// don't send empty messages
						if (messageField.text == "") {
							return;
						}

						// disable the button to prevent sending
						// the same message several times
						sendButton.enabled = false;

						// send the message
						kaidan.sendMessage(recipientJid, messageField.text);
						// clean up the text field
						messageField.text = "";

						// reenable the button
						sendButton.enabled = true;
					}
				}
			}
		}
	}
}
