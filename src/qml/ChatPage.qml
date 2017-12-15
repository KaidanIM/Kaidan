/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2017 JBBgameich <jbb.mail@gmx.de>
 *  Copyright (C) 2017 Ilya Bizyaev <bizyaev@zoho.com>
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
import "elements"

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

			delegate: ChatMessage {
				sentByMe: model.recipient !== kaidan.jid
				messageBody: model.message
				dateTime: new Date(timestamp)
				isRead: model.isDelivered
				recipientAvatarUrl: kaidan.avatarStorage.getHashOfJid(author) !== "" ?
						    kaidan.avatarStorage.getAvatarUrl(author) :
						    kaidan.getResourcePath("images/fallback-avatar.svg")
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

				Controls.TextField {
					id: messageField
					Layout.fillWidth: true
					placeholderText: qsTr("Compose message")
					wrapMode: Controls.TextArea.Wrap
					selectByMouse: true
					onAccepted: {
						sendButton.onClicked()
					}
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
