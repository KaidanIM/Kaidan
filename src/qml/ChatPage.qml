/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

import QtQuick 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtMultimedia 5.12 as Multimedia
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

import "elements"

ChatPageBase {
	id: root

	DropArea {
		anchors.fill: parent
		onDropped: (drop) => {
			if (drop.urls.length > 0) {
				sendMediaSheet.sendFile(Kaidan.messageModel.currentChatJid, drop.urls[0])
			}
		}
	}

	Shortcut {
		sequence: "Ctrl+Shift+V"
		context: Qt.WindowShortcut
		onActivated: {
			var imageUrl = Utils.pasteImage();
			// check if there was an image to be pasted from the clipboard
			if (imageUrl.toString().length > 0) {
				sendMediaSheet.sendFile(Kaidan.messageModel.currentChatJid, imageUrl)
			}
		}
	}

	property string chatName: {
		var currentChatJid = Kaidan.messageModel.currentChatJid
		var chatDisplayName = Kaidan.rosterModel.itemName(currentChatJid)
		return chatDisplayName ? chatDisplayName : currentChatJid
	}

	property alias mediaDrawer: mediaDrawer
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
			icon.name: "system-search-symbolic"

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
				Kaidan.notificationsMuted(Kaidan.messageModel.currentChatJid)
					? "audio-volume-high-symbolic"
					: "audio-volume-muted-symbolic"
			}
			text: {
				Kaidan.notificationsMuted(Kaidan.messageModel.currentChatJid)
					? qsTr("Unmute notifications")
					: qsTr("Mute notifications")
			}
			onTriggered: {
				Kaidan.setNotificationsMuted(
					Kaidan.messageModel.currentChatJid,
					!Kaidan.notificationsMuted(Kaidan.messageModel.currentChatJid)
				)
			}

			Connections {
				target: Kaidan

				function onNotificationsMuted(jid) {
					text = Kaidan.notificationsMuted(Kaidan.messageModel.currentChatJid)
							? qsTr("Unmute notifications")
							: qsTr("Mute notifications")
					icon.name = Kaidan.notificationsMuted(Kaidan.messageModel.currentChatJid)
							    ? "audio-volume-high-symbolic"
								: "audio-volume-muted-symbolic"
				}
			}
		},
		Kirigami.Action {
			visible: true
			icon.name: "avatar-default-symbolic"
			text: qsTr("View profile")
			onTriggered: pageStack.push(userProfilePage, {jid: Kaidan.messageModel.currentChatJid, name: chatName})
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageImage

			text: MediaUtilsInstance.newMediaLabel(type)
			enabled: root.cameraAvailable

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.currentChatJid, type)
			}
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageAudio

			text: MediaUtilsInstance.newMediaLabel(type)

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.currentChatJid, type)
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
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.currentChatJid, type)
			}
		},
		Kirigami.Action {
			readonly property int type: Enums.MessageType.MessageGeoLocation

			text: MediaUtilsInstance.newMediaLabel(type)

			icon {
				name: MediaUtilsInstance.newMediaIconName(type)
			}

			onTriggered: {
				sendMediaSheet.sendNewMessageType(Kaidan.messageModel.currentChatJid, type)
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
				text: qsTr("Close message search bar")
				icon.name: "window-close-symbolic"
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

	Loader {
		id: fileChooserLoader
	}

	function openFileDialog(nameFilter, title) {
		fileChooserLoader.source = "qrc:/qml/elements/FileChooser.qml"
		fileChooserLoader.item.selectedNameFilter = nameFilter
		fileChooserLoader.item.accepted.connect(
			function() {
				sendMediaSheet.sendFile(Kaidan.messageModel.currentChatJid, fileChooserLoader.item.fileUrl)
			}
		)
		if (title !== undefined)
			fileChooserLoader.item.title = title
		fileChooserLoader.item.open()
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

			delegate: Controls.ToolButton {
				height: ListView.view.height
				width: height
				text: MediaUtilsInstance.label(model.modelData)
				icon {
					name: MediaUtilsInstance.iconName(model.modelData)
					height: Kirigami.Units.gridUnit * 5
					width: height
				}
				display: Controls.AbstractButton.TextUnderIcon

				onClicked: {
					switch (model.modelData) {
					case Enums.MessageType.MessageFile:
					case Enums.MessageType.MessageImage:
					case Enums.MessageType.MessageAudio:
					case Enums.MessageType.MessageVideo:
					case Enums.MessageType.MessageDocument:
						openFileDialog(MediaUtilsInstance.namedFilter(model.modelData),
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

	// button for jumping to the latest message
	Controls.RoundButton {
		visible: width > 0
		width: messageListView.atYEnd ? 0 : 50
		height: messageListView.atYEnd ? 0 : 50
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: sendingPane.height + 5
		icon.name: "go-down"
		onClicked: messageListView.positionViewAtIndex(0, ListView.Center)

		Behavior on width {
			SmoothedAnimation {}
		}

		Behavior on height {
			SmoothedAnimation {}
		}
	}

	// View containing the messages
	ListView {
		id: messageListView
		verticalLayoutDirection: ListView.BottomToTop
		spacing: Kirigami.Units.smallSpacing * 1.5

		// placeholder
		footer: Item {
			height: Kirigami.Units.smallSpacing * 4
		}

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

		ChatMessageContextMenu {
			id: messageContextMenu
		}

		delegate: ChatMessage {
			modelIndex: index
			msgId: model.id
			senderJid: model.sender
			senderName: chatName
			contextMenu: messageContextMenu
			sentByMe: model.sentByMe
			messageBody: model.body
			dateTime: new Date(model.timestamp)
			deliveryState: model.deliveryState
			mediaType: model.mediaType
			mediaGetUrl: model.mediaUrl
			mediaLocation: model.mediaLocation
			edited: model.isEdited
			isSpoiler: model.isSpoiler
			isShowingSpoiler: false
			spoilerHint: model.spoilerHint
			errorText: model.errorText
			deliveryStateName: model.deliveryStateName
			deliveryStateIcon: model.deliveryStateIcon

			onMessageEditRequested: {
				messageToCorrect = id

				sendingPane.messageArea.text = body
				sendingPane.messageArea.state = "edit"
			}

			onQuoteRequested: {
				var quotedText = ""
				var lines = body.split("\n")

				for (var line in lines) {
					quotedText += "> " + lines[line] + "\n"
				}

				sendingPane.messageArea.insert(0, quotedText)
			}
		}
	}

	footer: ChatPageSendingPane {
		id: sendingPane
		chatPage: root
	}
}
