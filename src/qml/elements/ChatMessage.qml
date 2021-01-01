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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtGraphicalEffects 1.12
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

RowLayout {
	id: root

	property Controls.Menu contextMenu

	property int modelIndex
	property string msgId
	property string senderJid
	property string senderName
	property bool sentByMe: true
	property string messageBody
	property date dateTime
	property int deliveryState: Enums.DeliveryState.Delivered
	property int mediaType
	property string mediaGetUrl
	property string mediaLocation
	property bool edited
	property bool isLoading: Kaidan.transferCache.hasUpload(msgId)
	property TransferJob upload: {
		if (mediaType !== Enums.MessageType.MessageText && isLoading) {
			return Kaidan.transferCache.jobByMessageId(model.id)
		}

		return null
	}
	property bool isSpoiler
	property string spoilerHint
	property bool isShowingSpoiler: false
	property string avatarUrl: Kaidan.avatarStorage.getAvatarUrl(senderJid)
	property string errorText: ""
	property alias bodyLabel: bodyLabel
	property string deliveryStateName
	property url deliveryStateIcon

	signal messageEditRequested(string id, string body)
	signal quoteRequested(string body)

	// Own messages are on the right, others on the left side.
	layoutDirection: sentByMe ? Qt.RightToLeft : Qt.LeftToRight
	spacing: 8
	width: ListView.view.width

	// placeholder
	Item {
		Layout.preferredWidth: 5
	}

	Avatar {
		id: avatar
		visible: !sentByMe
		avatarUrl: root.avatarUrl
		Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
		name: root.senderName
		Layout.preferredHeight: Kirigami.Units.gridUnit * 2.2
		Layout.preferredWidth: Kirigami.Units.gridUnit * 2.2
	}

	// message bubble
	Item {
		Layout.preferredWidth: content.width + 16
		Layout.preferredHeight: content.height + 16

		// inner area of the message bubble
		Kirigami.ShadowedRectangle {
			id: messageBubble
			anchors.fill: parent
			shadow.color: Qt.darker(color, 1.2)
			shadow.size: 4
			radius: roundedCornersRadius
			color: sentByMe ? rightMessageBubbleColor : leftMessageBubbleColor

			MouseArea {
				anchors.fill: parent
				acceptedButtons: Qt.LeftButton | Qt.RightButton

				onClicked: {
					if (mouse.button === Qt.RightButton)
						showContextMenu()
				}

				onPressAndHold: showContextMenu()
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
						print("Downloading " + mediaGetUrl + "…")
						Kaidan.downloadMedia(msgId, mediaGetUrl)
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
					visible: messageBody
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

			// message meta data: date, deliveryState
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
					visible: sentByMe
					source: deliveryStateIcon
					Layout.preferredHeight: Kirigami.Units.gridUnit * 0.65
					Layout.preferredWidth: Kirigami.Units.gridUnit * 0.65
					sourceSize.height: Kirigami.Units.gridUnit * 0.65
					sourceSize.width: Kirigami.Units.gridUnit * 0.65

					MouseArea {
						id: checkmarkMouseArea
						anchors.fill: parent
						hoverEnabled: true
					}

					Controls.ToolTip {
						text: deliveryStateName
						visible: checkmarkMouseArea.containsMouse
						delay: 500
					}
				}

				Kirigami.Icon {
					source: "document-edit-symbolic"
					visible: edited
					Layout.preferredHeight: Kirigami.Units.gridUnit * 0.65
					Layout.preferredWidth: Kirigami.Units.gridUnit * 0.65
				}
			}

			Controls.Label {
				visible: errorText
				id: errorLabel
				text: qsTr(errorText)
				color: Kirigami.Theme.disabledTextColor
				font.pixelSize: Kirigami.Units.gridUnit * 0.8
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

	/**
	 * Shows a context menu (if available) for this message.
	 *
	 * That is especially the case when this message is an element of the ChatPage.
	 */
	function showContextMenu() {
		if (contextMenu) {
			contextMenu.message = this
			contextMenu.popup()
		}
	}

	function updateIsLoading() {
		isLoading = Kaidan.transferCache.hasUpload(msgId)
	}

	Component.onCompleted: {
		Kaidan.transferCache.jobsChanged.connect(updateIsLoading)
	}
	Component.onDestruction: {
		Kaidan.transferCache.jobsChanged.disconnect(updateIsLoading)
	}
}
