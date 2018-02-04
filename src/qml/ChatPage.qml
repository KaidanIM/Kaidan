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
import QtQuick 2.6
import QtQuick.Controls 2.0 as Controls
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami
import QtGraphicalEffects 1.0
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

	background: Image {
		id: bgimage
		source: kaidan.getResourcePath("images/chat.png")
		anchors.fill: parent
		fillMode: Image.Tile
		horizontalAlignment: Image.AlignLeft
		verticalAlignment: Image.AlignTop
	}

	//
	// Chat
	//
	ColumnLayout {
		anchors.fill: parent
		ListView {
			Layout.fillWidth: true
			Layout.fillHeight: true
			bottomMargin: 20

			verticalLayoutDirection: ListView.BottomToTop

			spacing: 12

			// connect the database
			model: kaidan.messageModel

			delegate: ChatMessage {
				width: parent.width
				sentByMe: model.recipient !== kaidan.jid
				messageBody: model.message
				dateTime: new Date(timestamp)
				isRead: model.isDelivered
				recipientAvatarUrl: kaidan.avatarStorage.getHashOfJid(author) !== "" ?
									kaidan.avatarStorage.getAvatarUrl(author) :
									kaidan.getResourcePath("images/fallback-avatar.svg")
			}
			Controls.ScrollIndicator.vertical: Controls.ScrollIndicator {}
		}
	}

	//
	// Message Writing
	//
	footer: Controls.Pane {
		id: sendingArea
		layer.enabled: sendingArea.enabled
		layer.effect: DropShadow {
		  verticalOffset: 1
		  color: Material.dropShadowColor
		  samples: 20
		  spread: 0.3
		}
		Layout.fillWidth: true
		padding: 0
		leftPadding: 12
		wheelEnabled: true
		background: Rectangle {
			color: "white"
		}

		RowLayout {
			width: parent.width

			/*Controls.ToolButton {
				id: attachButton
				Layout.preferredWidth: 60
				Layout.preferredHeight: 60
				Kirigami.Icon {
					source: "document-send-symbolic"
					isMask: true
					smooth: true
					anchors.centerIn: parent
					width: 30
					height: width
				}
			}*/

			Controls.TextArea {
				id: messageField
				Layout.fillWidth: true
				placeholderText: qsTr("Compose message")
				wrapMode: Controls.TextArea.Wrap
				topPadding: 10
				bottomPadding: 10
				selectByMouse: true
				background: Item {}
				Keys.onReturnPressed: {
					if (event.key === Qt.Key_Return) {
						if (event.modifiers & Qt.ControlModifier) {
							messageField.append("")
						} else {
							sendButton.onClicked()
							event.accepted = true
						}
					}
				}
			}

			Controls.ToolButton {
				id: sendButton
				Layout.preferredWidth: 60
				Layout.preferredHeight: 60
				//enabled: messageField.text.length - only counts spaces?
				Kirigami.Icon {
					source: "document-send"
					enabled: sendButton.enabled
					isMask: true
					smooth: true
					anchors.centerIn: parent
					width: 30
					height: width
				}
				onClicked: {
					// don't send empty messages
					if (!messageField.text.length) {
						return
					}

					// disable the button to prevent sending
					// the same message several times
					sendButton.enabled = false

					// send the message
					kaidan.sendMessage(recipientJid, messageField.text)
					// clean up the text field
					messageField.text = ""

					// reenable the button
					sendButton.enabled = true
				}
			}
		}
	}
}
