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

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.0 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

RowLayout {
	id: root

	property string msgId
	property string sender
	property bool sentByMe: true
	property string messageBody
	property date dateTime
	property bool isDelivered: false
	property int mediaType
	property string mediaGetUrl
	property string mediaLocation
	property bool edited
	property bool isLoading: kaidan.transferCache.hasUpload(msgId)
	property string name
	property TransferJob upload: {
		if (mediaType !== Enums.MessageType.MessageText && isLoading) {
			return kaidan.transferCache.jobByMessageId(model.id)
		}

		return null
	}
	property bool isSpoiler
	property string spoilerHint
	property bool isShowingSpoiler: false
	property string avatarUrl: kaidan.avatarStorage.getAvatarUrl(sender)

	signal messageEditRequested(string id, string body)

	// Own messages are on the right, others on the left side.
	layoutDirection: sentByMe ? Qt.RightToLeft : Qt.LeftToRight
	spacing: 8
	width: ListView.view.width

	// placeholder
	Item {
		Layout.preferredWidth: root.layoutDirection === Qt.LeftToRight ? 5 : 10
	}

	Avatar {
		id: avatar
		visible: !sentByMe
		avatarUrl: root.avatarUrl
		Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
		name: root.name
		Layout.preferredHeight: Kirigami.Units.gridUnit * 2.2
		Layout.preferredWidth: Kirigami.Units.gridUnit * 2.2
	}

	// message bubble
	Item {
		Layout.preferredWidth: content.width + 16
		Layout.preferredHeight: content.height + 16

		// glow effect around the inner area of the message bubble
		RectangularGlow {
			anchors.fill: messageBubble
			glowRadius: 0.8
			spread: 0.3
			cornerRadius: messageBubble.radius + glowRadius
			color: Qt.darker(messageBubble.color, 1.2)
		}

		// inner area of the message bubble
		Rectangle {
			id: messageBubble
			anchors.fill: parent
			radius: roundedCornersRadius
			color: sentByMe ? rightMessageBubbleColor : leftMessageBubbleColor

			MouseArea {
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton | Qt.RightButton
				onClicked: {
					if (mouse.button === Qt.RightButton)
						contextMenu.popup()
				}
				onPressAndHold: {
					contextMenu.popup()
				}
			}

			Controls.Menu {
				id: contextMenu
				Controls.MenuItem {
					text: qsTr("Copy Message")
					enabled: bodyLabel.visible
					onTriggered: {
						if (!isSpoiler || isShowingSpoiler)
							Utils.copyToClipboard(messageBody);
						else
							Utils.copyToClipboard(spoilerHint);
					}
				}

				Controls.MenuItem {
					text: qsTr("Edit Message")
					enabled: kaidan.messageModel.canCorrectMessage(msgId)
					onTriggered: root.messageEditRequested(msgId, messageBody)
				}

				Controls.MenuItem {
					text: qsTr("Copy download URL")
					enabled: mediaGetUrl
					onTriggered: Utils.copyToClipboard(mediaGetUrl)
				}
			}
		}

		ColumnLayout {
			id: content
			spacing: 5
			anchors.centerIn: parent

			RowLayout {
				id: spoilerHintRow
				visible: isSpoiler

				Controls.Label {
					text: spoilerHint == "" ? qsTr("Spoiler") : spoilerHint
					color: Kirigami.Theme.textColor
					font.pixelSize: Kirigami.Units.gridUnit * 0.8
					MouseArea {
						anchors.fill: parent
						acceptedButtons: Qt.LeftButton | Qt.RightButton
						onClicked: {
							if (mouse.button === Qt.LeftButton) {
								isShowingSpoiler = !isShowingSpoiler
							}
						}
					}
				}

				Item {
					Layout.fillWidth: true
					height: 1
				}

				Kirigami.Icon {
					height: 28
					width: 28
					source: isShowingSpoiler ? "password-show-off" : "password-show-on"
					color: Kirigami.Theme.textColor
				}
			}
			Kirigami.Separator {
				visible: isSpoiler
				Layout.fillWidth: true
				color: {
					var bgColor = Kirigami.Theme.backgroundColor
					var textColor = Kirigami.Theme.textColor
					return Qt.tint(textColor, Qt.rgba(bgColor.r, bgColor.g, bgColor.b, 0.7))
				}
			}

			ColumnLayout {
				visible: isSpoiler && isShowingSpoiler || !isSpoiler

				Controls.ToolButton {
					visible: {
						switch (root.mediaType) {
						case Enums.MessageType.MessageUnknown:
						case Enums.MessageType.MessageText:
						case Enums.MessageType.MessageGeoLocation:
							break
						case Enums.MessageType.MessageImage:
						case Enums.MessageType.MessageAudio:
						case Enums.MessageType.MessageVideo:
						case Enums.MessageType.MessageFile:
						case Enums.MessageType.MessageDocument:
							return !root.isLoading && root.mediaGetUrl !== ""
									&& (root.mediaLocation === "" || !MediaUtilsInstance.localFileAvailable(media.mediaSource))
						}

						return false
					}
					text: qsTr("Download")
					onClicked: {
						print("Downloading " + mediaGetUrl + "...")
						kaidan.downloadMedia(msgId, mediaGetUrl)
					}
				}

				MediaPreviewLoader {
					id: media

					mediaSource: {
						switch (root.mediaType) {
						case Enums.MessageType.MessageUnknown:
						case Enums.MessageType.MessageText:
							break
						case Enums.MessageType.MessageGeoLocation:
							return root.mediaLocation
						case Enums.MessageType.MessageImage:
						case Enums.MessageType.MessageAudio:
						case Enums.MessageType.MessageVideo:
						case Enums.MessageType.MessageFile:
						case Enums.MessageType.MessageDocument:
							const localFile = root.mediaLocation !== ''
											? MediaUtilsInstance.fromLocalFile(root.mediaLocation)
											: ''
							return MediaUtilsInstance.localFileAvailable(localFile) ? localFile : root.mediaGetUrl
						}

						return ''
					}
					mediaSourceType: root.mediaType
					showOpenButton: true
					message: root
				}

				// message body
				Controls.Label {
					id: bodyLabel
					visible: (root.mediaType === Enums.MessageType.MessageText || messageBody !== mediaGetUrl) && messageBody !== ""
					text: Utils.formatMessage(messageBody)
					textFormat: Text.StyledText
					wrapMode: Text.Wrap
					color: Kirigami.Theme.textColor
					onLinkActivated: Qt.openUrlExternally(link)

					Layout.maximumWidth: media.enabled
										? media.width
										: root.width - Kirigami.Units.gridUnit * 6
				}
				Kirigami.Separator {
					visible: isSpoiler && isShowingSpoiler
					Layout.fillWidth: true
					color: {
						var bgColor = Kirigami.Theme.backgroundColor
						var textColor = Kirigami.Theme.textColor
						return Qt.tint(textColor, Qt.rgba(bgColor.r, bgColor.g, bgColor.b, 0.7))
					}
				}
			}

			// message meta data: date, isDelivered
			RowLayout {
				Layout.bottomMargin: -4

				Controls.Label {
					id: dateLabel
					text: Qt.formatDateTime(dateTime, "dd. MMM yyyy, hh:mm")
					color: Kirigami.Theme.disabledTextColor
					font.pixelSize: Kirigami.Units.gridUnit * 0.8
				}

				Image {
					id: checkmark
					visible: (sentByMe && isDelivered)
					source: Utils.getResourcePath("images/message_checkmark.svg")
					Layout.preferredHeight: Kirigami.Units.gridUnit * 0.65
					Layout.preferredWidth: Kirigami.Units.gridUnit * 0.65
					sourceSize.height: Kirigami.Units.gridUnit * 0.65
					sourceSize.width: Kirigami.Units.gridUnit * 0.65
				}
				Kirigami.Icon {
					source: "edit-symbolic"
					visible: edited
					Layout.preferredHeight: Kirigami.Units.gridUnit * 0.65
					Layout.preferredWidth: Kirigami.Units.gridUnit * 0.65
				}
			}

			// progress bar for upload/download status
			Controls.ProgressBar {
				visible: isLoading
				value: upload ? upload.progress : 0

				Layout.fillWidth: true
				Layout.maximumWidth: Kirigami.Units.gridUnit * 14
			}
		}
	}

	// placeholder
	Item {
		Layout.fillWidth: true
	}

	function updateIsLoading() {
		isLoading = kaidan.transferCache.hasUpload(msgId)
	}

	Component.onCompleted: {
		kaidan.transferCache.jobsChanged.connect(updateIsLoading)
	}
	Component.onDestruction: {
		kaidan.transferCache.jobsChanged.disconnect(updateIsLoading)
	}
}
