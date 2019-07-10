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

import QtQuick 2.6
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.2 as Kirigami
import QtGraphicalEffects 1.0
import im.kaidan.kaidan 1.0
import EmojiModel 0.1
import "elements"

Kirigami.ScrollablePage {
	property string chatName
	property bool isWritingSpoiler
	property string messageToCorrect

	title: chatName
	keyboardNavigationEnabled: true
	actions.contextualActions: [
		Kirigami.Action {
			visible: !isWritingSpoiler
			iconSource: "password-show-off"
			text: qsTr("Send a spoiler message")
			onTriggered: isWritingSpoiler = true
		}
	]

	SendMediaSheet {
		id: sendMediaSheet
	}

	FileChooser {
		id: fileChooser
		title: qsTr("Select a file")
		onAccepted: {
			sendMediaSheet.jid = kaidan.messageModel.chatPartner
			sendMediaSheet.fileUrl = fileUrl
			sendMediaSheet.open()
		}
	}

	function openFileDialog(filterName, filter) {
		fileChooser.filterName = filterName
		fileChooser.filter = filter
		fileChooser.open()
		mediaDrawer.close()
	}

	Kirigami.OverlayDrawer {
		id: mediaDrawer
		edge: Qt.BottomEdge
		height: Kirigami.Units.gridUnit * 8
		contentItem: RowLayout {
			id: content
			Layout.alignment: Qt.AlignHCenter
			Layout.fillHeight: true

			IconButton {
				buttonText: qsTr("Image")
				iconSource: "image-jpeg"
				onClicked: openFileDialog("Images", "*.jpg *.jpeg *.png *.gif")
				Layout.alignment: Qt.AlignHCenter
			}
			IconButton {
				buttonText: qsTr("Video")
				iconSource: "video-mp4"
				onClicked: openFileDialog("Videos", "*.mp4 *.mkv *.avi *.webm")
				Layout.alignment: Qt.AlignHCenter
			}
			IconButton {
				buttonText: qsTr("Audio")
				iconSource: "audio-mp3"
				onClicked: openFileDialog("Audio files", "*.mp3 *.wav *.flac *.ogg *.m4a *.mka")
				Layout.alignment: Qt.AlignHCenter
			}
			IconButton {
				buttonText: qsTr("Document")
				iconSource: "x-office-document"
				onClicked: openFileDialog("Documents", "*.doc *.docx *.odt")
				Layout.alignment: Qt.AlignHCenter
			}
			IconButton {
				buttonText: qsTr("Other file")
				iconSource: "text-x-plain"
				onClicked: openFileDialog("All files", "*")
				Layout.alignment: Qt.AlignHCenter
			}

		}
	}

	background: Image {
		id: bgimage
		source: kaidan.utils.getResourcePath("images/chat.png")
		anchors.fill: parent
		fillMode: Image.Tile
		horizontalAlignment: Image.AlignLeft
		verticalAlignment: Image.AlignTop
	}

	// Chat
	ListView {
		verticalLayoutDirection: ListView.BottomToTop
		spacing: Kirigami.Units.smallSpacing * 2

		// connect the database
		model: kaidan.messageModel

		delegate: ChatMessage {
			msgId: model.id
			sender: model.sender
			sentByMe: model.sentByMe
			messageBody: model.body
			dateTime: new Date(model.timestamp)
			isRead: model.isDelivered
			name: chatName
			mediaType: model.mediaType
			mediaGetUrl: model.mediaUrl
			mediaLocation: model.mediaLocation
			edited: model.isEdited
			isSpoiler: model.isSpoiler
			isShowingSpoiler: false
			spoilerHint: model.spoilerHint

			onMessageEditRequested: {
				messageToCorrect = id
				messageField.text = body
				messageField.state = "edit"
			}
		}
	}

	// Message Writing
	footer: Controls.Pane {
		id: sendingArea
		layer.enabled: sendingArea.enabled
		layer.effect: DropShadow {
			verticalOffset: 1
			color: Kirigami.Theme.disabledTextColor
			samples: 20
			spread: 0.3
			cached: true // element is static
		}
		padding: 0
		wheelEnabled: true
		background: Rectangle {
			color: Kirigami.Theme.backgroundColor
		}

		RowLayout {
			anchors.fill: parent
			Layout.preferredHeight: Kirigami.Units.gridUnit * 3

			Controls.ToolButton {
				id: attachButton
				visible: kaidan.uploadServiceFound
				Layout.preferredWidth: Kirigami.Units.gridUnit * 3
				Layout.preferredHeight: Kirigami.Units.gridUnit * 3
				padding: 0
				Kirigami.Icon {
					source: "document-send-symbolic"
					isMask: true
					smooth: true
					anchors.centerIn: parent
					width: Kirigami.Units.gridUnit * 2
					height: width
				}
				onClicked: {
					if (Kirigami.Settings.isMobile)
						mediaDrawer.open()
					else
						openFileDialog("All files", "(*)")
				}
			}

			ColumnLayout {
				Layout.minimumHeight: messageField.height + Kirigami.Units.smallSpacing * 2
				Layout.fillWidth: true
				spacing: 0
				RowLayout {
					visible: isWritingSpoiler
					Controls.TextArea {
						id: spoilerHintField
						Layout.fillWidth: true
						placeholderText: qsTr("Spoiler hint")
						wrapMode: Controls.TextArea.Wrap
						selectByMouse: true
						background: Item {}
					}
					Controls.ToolButton {
						Layout.preferredWidth: Kirigami.Units.gridUnit * 1.5
						Layout.preferredHeight: Kirigami.Units.gridUnit * 1.5
						padding: 0
						Kirigami.Icon {
							source: "tab-close"
							smooth: true
							anchors.centerIn: parent
							width: Kirigami.Units.gridUnit * 1.5
							height: width
						}
						onClicked: {
							isWritingSpoiler = false
							spoilerHintField.text = ""
						}
					}
				}
				Kirigami.Separator {
					visible: isWritingSpoiler
					Layout.fillWidth: true
				}
				Controls.TextArea {
					id: messageField

					Layout.fillWidth: true
					Layout.alignment: Qt.AlignVCenter
					placeholderText: qsTr("Compose message")
					wrapMode: Controls.TextArea.Wrap
					selectByMouse: true
					background: Item {}
					state: "compose"
					states: [
						State {
							name: "compose"
						},
						State {
							name: "edit"
						}
					]
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
			}

			EmojiPicker {
				x: -width + parent.width
				y: -height - 16

				width: Kirigami.Units.gridUnit * 20
				height: Kirigami.Units.gridUnit * 15

				id: emojiPicker

				model: EmojiProxyModel {
					group: Emoji.Group.People
					sourceModel: EmojiModel {}
				}

				textArea: messageField
			}

			Controls.ToolButton {
				id: emojiPickerButton
				Layout.preferredWidth: Kirigami.Units.gridUnit * 3
				Layout.preferredHeight: Kirigami.Units.gridUnit * 3
				padding: 0
				Kirigami.Icon {
					source: "face-smile"
					enabled: sendButton.enabled
					isMask: false
					smooth: true
					anchors.centerIn: parent
					width: Kirigami.Units.gridUnit * 2
					height: width
				}
				onClicked: emojiPicker.visible ? emojiPicker.close() : emojiPicker.open()
			}

			Controls.ToolButton {
				id: sendButton
				Layout.preferredWidth: Kirigami.Units.gridUnit * 3
				Layout.preferredHeight: Kirigami.Units.gridUnit * 3
				padding: 0
				Kirigami.Icon {
					source: {
						if (messageField.state == "compose")
							return "document-send"
						else if (messageField.state == "edit")
							return "edit-symbolic"
					}
					enabled: sendButton.enabled
					isMask: true
					smooth: true
					anchors.centerIn: parent
					width: Kirigami.Units.gridUnit * 2
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
					if (messageField.state == "compose") {
						kaidan.sendMessage(
							kaidan.messageModel.chatPartner,
							messageField.text,
							isWritingSpoiler,
							spoilerHintField.text
						)
					} else if (messageField.state == "edit") {
						kaidan.correctMessage(
							kaidan.messageModel.chatPartner,
							messageToCorrect,
							messageField.text
						)
					}

					// clean up the text fields
					messageField.text = ""
					messageField.state = "compose"
					spoilerHintField.text = ""
					isWritingSpoiler = false

					// reenable the button
					sendButton.enabled = true
				}
			}
		}
	}
}
