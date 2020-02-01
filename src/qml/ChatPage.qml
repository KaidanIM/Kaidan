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

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import QtGraphicalEffects 1.0
import QtMultimedia 5.8 as Multimedia
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0
import EmojiModel 0.1
import MediaUtils 0.1

import "elements"

ChatPageBase {
	id: root

	property string chatName
	property bool isWritingSpoiler
	property string messageToCorrect

	readonly property bool cameraAvailable: Multimedia.QtMultimedia.availableCameras.length > 0

	title: chatName
	keyboardNavigationEnabled: true
	contextualActions: [
		// Action to toggle the message search bar
		Kirigami.Action {
			id: searchAction
			text: qsTr("Search")
			icon.name: "search"

			onTriggered: {
				if (searchBar.active)
					searchBar.close()
				else
					searchBar.open()
			}
		},
		Kirigami.Action {
			visible: true
			icon.name: {
				Kaidan.notificationsMuted(Kaidan.messageModel.chatPartner)
					? "player-volume"
					: "audio-volume-muted-symbolic"
			}
			text: {
				Kaidan.notificationsMuted(Kaidan.messageModel.chatPartner)
					? qsTr("Unmute notifications")
					: qsTr("Mute notifications")
			}
			onTriggered: {
				Kaidan.setNotificationsMuted(
					Kaidan.messageModel.chatPartner,
					!Kaidan.notificationsMuted(Kaidan.messageModel.chatPartner)
				)
			}

			function handleNotificationsMuted(jid) {
				text = Kaidan.notificationsMuted(Kaidan.messageModel.chatPartner)
						? qsTr("Unmute notifications")
						: qsTr("Mute notifications")
				icon.name = Kaidan.notificationsMuted(Kaidan.messageModel.chatPartner)
							? "player-volume"
							: "audio-volume-muted-symbolic"
			}

			Component.onCompleted: {
				Kaidan.notificationsMutedChanged.connect(handleNotificationsMuted)
			}
			Component.onDestruction: {
				Kaidan.notificationsMutedChanged.disconnect(handleNotificationsMuted)
			}
		},
		Kirigami.Action {
			visible: true
			icon.name: "user-identity"
			text: qsTr("View profile")
			onTriggered: pageStack.push(userProfilePage, {jid: Kaidan.messageModel.chatPartner, name: chatName})
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageImage

			text: MediaUtilsInstance.newMediaLabel(type)
			enabled: root.cameraAvailable

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.chatPartner, type)
			}
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageAudio

			text: MediaUtilsInstance.newMediaLabel(type)

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.chatPartner, type)
			}
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageVideo

			text: MediaUtilsInstance.newMediaLabel(type)
			enabled: root.cameraAvailable

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.chatPartner, type)
			}
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageGeoLocation

			text: MediaUtilsInstance.newMediaLabel(type)

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.chatPartner, type)
			}
		},
		Kirigami.Action {
			visible: !isWritingSpoiler
			icon.name: "password-show-off"
			text: qsTr("Send a spoiler message")
			onTriggered: isWritingSpoiler = true
		}
	]

	// Message search bar
	header: Item {
		id: searchBar
		height: active ? searchField.height + 2 * Kirigami.Units.largeSpacing : 0
		clip: true
		visible: height != 0
		property bool active: false

		Behavior on height {
			SmoothedAnimation {
				velocity: 200
			}
		}

		// Background of the message search bar
		Rectangle {
			anchors.fill: parent
			color: Kirigami.Theme.backgroundColor
		}

		/**
		 * Searches for a message containing the entered text in the search field.
		 *
		 * If a message is found for the entered text, that message is highlighted.
		 * If the upwards search reaches the top of the message list view, the search is restarted at the bottom to search for messages which were not included in the search yet because they were below the message at the start index.
		 * That behavior is not applied to an upwards search starting from the index of the most recent message (0) to avoid searching twice.
		 * If the downwards search reaches the bottom of the message list view, the search is restarted at the top to search for messages which were not included in the search yet because they were above the message at the start index.
		 *
		 * @param searchUpwards true for searching upwards or false for searching downwards
		 * @param startIndex index index of the first message to search for the entered text
		 */
		function search(searchUpwards, startIndex) {
			let newIndex = -1
			if (searchBar.active && searchField.text.length > 0) {
				if (searchUpwards) {
					if (startIndex === 0) {
						newIndex = Kaidan.messageModel.searchForMessageFromNewToOld(searchField.text)
					} else {
						newIndex = Kaidan.messageModel.searchForMessageFromNewToOld(searchField.text, startIndex)
						if (newIndex === -1)
							newIndex = Kaidan.messageModel.searchForMessageFromNewToOld(searchField.text, 0)
					}
				} else {
					newIndex = Kaidan.messageModel.searchForMessageFromOldToNew(searchField.text, startIndex)
					if (newIndex === -1)
						newIndex = Kaidan.messageModel.searchForMessageFromOldToNew(searchField.text)
				}
			}
			messageListView.currentIndex = newIndex
		}

		/**
		 * Hides the search bar and resets the last search result.
		 */
		function close() {
			messageListView.currentIndex = -1
			searchBar.active = false
		}

		/**
		 * Shows the search bar and focuses the search field.
		 */
		function open() {
			searchField.forceActiveFocus()
			searchBar.active = true
		}

		/**
		 * Searches upwards for a message containing the entered text in the search field starting from the current index of the message list view.
		 */
		function searchUpwardsFromBottom() {
			searchBar.search(true, 0)
		}

		/**
		 * Searches upwards for a message containing the entered text in the search field starting from the current index of the message list view.
		 */
		function searchUpwardsFromCurrentIndex() {
			searchBar.search(true, messageListView.currentIndex + 1)
		}

		/**
		 * Searches downwards for a message containing the entered text in the search field starting from the current index of the message list view.
		 */
		function searchDownwardsFromCurrentIndex() {
			searchBar.search(false, messageListView.currentIndex - 1)
		}

		// Search field and ist corresponding buttons
		RowLayout {
			// Anchoring like this binds it to the top of the chat page.
			// It makes it look like the search bar slides down from behind of the upper element.
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.margins: Kirigami.Units.largeSpacing

			Controls.Button {
				text: qsTr("Close")
				icon.name: "dialog-close"
				onClicked: searchBar.close()
				display: Controls.Button.IconOnly
				flat: true
			}

			Kirigami.SearchField {
				id: searchField
				Layout.fillWidth: true
				focusSequence: ""
				onVisibleChanged: text = ""

				onTextChanged: searchBar.searchUpwardsFromBottom()
				onAccepted: searchBar.searchUpwardsFromCurrentIndex()
				Keys.onUpPressed: searchBar.searchUpwardsFromCurrentIndex()
				Keys.onDownPressed: searchBar.searchDownwardsFromCurrentIndex()
				Keys.onEscapePressed: searchBar.close()
			}

			Controls.Button {
				text: qsTr("Search up")
				icon.name: "go-up"
				display: Controls.Button.IconOnly
				flat: true
				onClicked: {
					searchBar.searchUpwardsFromCurrentIndex()
					searchField.forceActiveFocus()
				}
			}

			Controls.Button {
				text: qsTr("Search down")
				icon.name: "go-down"
				display: Controls.Button.IconOnly
				flat: true
				onClicked: {
					searchBar.searchDownwardsFromCurrentIndex()
					searchField.forceActiveFocus()
				}
			}
		}
	}

	SendMediaSheet {
		id: sendMediaSheet
	}

	FileChooser {
		id: fileChooser
		title: qsTr("Select a file")
		onAccepted: sendMediaSheet.sendFile(Kaidan.messageModel.chatPartner, fileUrl)
	}

	function openFileDialog(filterName, filter, title) {
		fileChooser.filterName = filterName
		fileChooser.filter = filter
		if (title !== undefined)
			fileChooser.title = title
		fileChooser.open()
		mediaDrawer.close()
	}

	Kirigami.OverlayDrawer {
		id: mediaDrawer

		edge: Qt.BottomEdge
		height: Kirigami.Units.gridUnit * 8

		contentItem: ListView {
			id: content

			orientation: Qt.Horizontal

			Layout.fillHeight: true
			Layout.fillWidth: true

			model: [
				Enums.MessageType.MessageFile,
				Enums.MessageType.MessageImage,
				Enums.MessageType.MessageAudio,
				Enums.MessageType.MessageVideo,
				Enums.MessageType.MessageDocument
			]

			delegate: IconButton {
				height: ListView.view.height
				width: height
				buttonText: MediaUtilsInstance.label(model.modelData)
				iconSource: MediaUtilsInstance.iconName(model.modelData)

				onClicked: {
					switch (model.modelData) {
					case Enums.MessageType.MessageFile:
					case Enums.MessageType.MessageImage:
					case Enums.MessageType.MessageAudio:
					case Enums.MessageType.MessageVideo:
					case Enums.MessageType.MessageDocument:
						openFileDialog(MediaUtilsInstance.filterName(model.modelData),
									   MediaUtilsInstance.filter(model.modelData),
									   MediaUtilsInstance.label(model.modelData))
						break
					case Enums.MessageType.MessageText:
					case Enums.MessageType.MessageGeoLocation:
					case Enums.MessageType.MessageUnknown:
						break
					}
				}
			}
		}
	}

	// View containing the messages
	ListView {
		id: messageListView
		verticalLayoutDirection: ListView.BottomToTop
		spacing: Kirigami.Units.smallSpacing * 1.5

		// Highlighting of the message containing a searched string.
		highlight: Component {
			id: highlightBar
			Rectangle {
				height: messageListView.currentIndex === -1 ? 0 : messageListView.currentItem.height + Kirigami.Units.smallSpacing * 2
				width: messageListView.currentIndex === -1 ? 0 : messageListView.currentItem.width + Kirigami.Units.smallSpacing * 2
				color: Kirigami.Theme.hoverColor

				// This is used to make the highlight bar a little bit bigger than the highlighted message.
				// It works only together with "messageListView.highlightFollowsCurrentItem: false".
				y: messageListView.currentIndex === -1 ? 0 : messageListView.currentItem.y - Kirigami.Units.smallSpacing
				x: messageListView.currentIndex === -1 ? 0 : messageListView.currentItem.x
				Behavior on y {
					SmoothedAnimation {
						velocity: 1000
						duration: 500
					}
				}

				Behavior on height {
					SmoothedAnimation {
						velocity: 1000
						duration: 500
					}
				}
			}
		}
		// This is used to make the highlight bar a little bit bigger than the highlighted message.
		highlightFollowsCurrentItem: false

		// Initially highlighted value
		currentIndex: -1

		// Connect to the database,
		model: Kaidan.messageModel

		delegate: ChatMessage {
			msgId: model.id
			sender: model.sender
			sentByMe: model.sentByMe
			messageBody: model.body
			dateTime: new Date(model.timestamp)
			isDelivered: model.isDelivered
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
			onQuoteRequested: {
				var quotedText = ""
				var lines = body.split("\n")

				for (var line in lines) {
					quotedText += "> " + lines[line] + "\n"
				}

				messageField.insert(0, quotedText)
			}
		}
	}

	// area for writing and sending a message
	footer: Controls.Pane {
		id: sendingArea
		padding: 0
		wheelEnabled: true

		background: Rectangle {
			id: sendingAreaBackground
			color: Kirigami.Theme.backgroundColor
		}

		layer.enabled: sendingArea.enabled
		layer.effect: DropShadow {
			verticalOffset: 1
			color: Qt.darker(sendingAreaBackground.color, 1.2)
			samples: 20
			spread: 0.3
			cached: true // element is static
		}

		RowLayout {
			anchors.fill: parent
			Layout.preferredHeight: Kirigami.Units.gridUnit * 3

			Controls.ToolButton {
				id: attachButton
				visible: Kaidan.uploadServiceFound
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
						openFileDialog(qsTr("All files"), "*", MediaUtilsInstance.label(Enums.MessageType.MessageFile))
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
						Kaidan.sendMessage(
							Kaidan.messageModel.chatPartner,
							messageField.text,
							isWritingSpoiler,
							spoilerHintField.text
						)
					} else if (messageField.state == "edit") {
						Kaidan.correctMessage(
							Kaidan.messageModel.chatPartner,
							messageToCorrect,
							messageField.text
						)
					}

					// clean up the text fields
					messageField.text = ""
					messageField.state = "compose"
					spoilerHintField.text = ""
					isWritingSpoiler = false
					messageToCorrect = ''

					// reenable the button
					sendButton.enabled = true
				}
			}
		}
	}
}
