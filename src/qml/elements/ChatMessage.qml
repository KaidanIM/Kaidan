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
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2 as Controls
import org.kde.kirigami 2.0 as Kirigami
import im.kaidan.kaidan 1.0

RowLayout {
	id: root

	property string msgId
	property bool sentByMe: true
	property string messageBody
	property date dateTime
	property bool isRead: false
	property string recipientAvatarUrl
	property int mediaType
	property string mediaGetUrl
	property string mediaLocation
	property bool edited
	property bool isLoading: kaidan.transferCache.hasUpload(msgId)
	property string name
	property var upload: {
		if (mediaType !== Enums.MessageText &&
			kaidan.transferCache.hasUpload(msgId)) {
			kaidan.transferCache.jobByMessageId(model.id)
		}
	}
	property bool isSpoiler
	property string spoilerHint
	property bool isShowingSpoiler: false

	signal messageEditRequested(string id, string body)

	// own messages are on the right, others on the left
	layoutDirection: sentByMe ? Qt.RightToLeft : Qt.LeftToRight
	spacing: 8
	width: parent.width

	// placeholder
	Item {
		Layout.preferredWidth: 5
	}

	RoundImage {
		id: avatar
		visible: !sentByMe && recipientAvatarUrl !== ""
		source: recipientAvatarUrl
		fillMode: Image.PreserveAspectFit
		mipmap: true
		height: width
		Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
		Layout.preferredHeight: Kirigami.Units.gridUnit * 2.2
		Layout.preferredWidth: Kirigami.Units.gridUnit * 2.2
		sourceSize.height: Kirigami.Units.gridUnit * 2.2
		sourceSize.width: Kirigami.Units.gridUnit * 2.2
	}

	TextAvatar {
		id: textAvatar
		visible: !sentByMe && recipientAvatarUrl == ""
		name: root.name
		Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
		Layout.preferredHeight: Kirigami.Units.gridUnit * 2.2
		Layout.preferredWidth: Kirigami.Units.gridUnit * 2.2
	}

	// message bubble/box
	Item {
		Layout.preferredWidth: content.width + 13
		Layout.preferredHeight: content.height + 8

		Rectangle {
			id: box
			anchors.fill: parent
			color: sentByMe ? Kirigami.Theme.complementaryTextColor
							: Kirigami.Theme.highlightColor
			radius: Kirigami.Units.smallSpacing * 2
			layer.enabled: box.visible
			layer.effect: DropShadow {
				verticalOffset: Kirigami.Units.gridUnit * 0.08
				horizontalOffset: Kirigami.Units.gridUnit * 0.08
				color: Kirigami.Theme.disabledTextColor
				samples: 10
				spread: 0.1
			}

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
					onTriggered: isShowingSpoiler ? kaidan.utils.copyToClipboard(messageBody) : kaidan.copyToClipboard(spoilerHint)
				}

				Controls.MenuItem {
					text: qsTr("Edit Message")
					enabled: kaidan.messageModel.canCorrectMessage(msgId)
					onTriggered: root.messageEditRequested(msgId, messageBody)
				}
			}
		}

		ColumnLayout {
			id: content
			spacing: 0
			anchors.centerIn: parent
			anchors.margins: 4
			RowLayout {
				id: spoilerHintRow
				visible: isSpoiler
				MouseArea {
					anchors.fill: parent
					acceptedButtons: Qt.LeftButton | Qt.RightButton
					onClicked: {
						if (mouse.button === Qt.LeftButton) {
							isShowingSpoiler = !isShowingSpoiler
						}
					}
				}
				Controls.Label {
					id: dateLabeltest
					text: spoilerHint == "" ? qsTr("Spoiler") : spoilerHint
					color: sentByMe ? Kirigami.Theme.buttonTextColor
								: Kirigami.Theme.complementaryTextColor
					font.pixelSize: Kirigami.Units.gridUnit * 0.8
				}

				Item {
					Layout.fillWidth: true
					height: 1
				}

				Kirigami.Icon {
					height: 28
					width: 28
					source: isShowingSpoiler ? "password-show-off" : "password-show-on"
					color: sentByMe ? Kirigami.Theme.buttonTextColor : Kirigami.Theme.complementaryTextColor
				}
			}
			Kirigami.Separator {
				visible: isSpoiler
				Layout.fillWidth: true
				color: {
					var bgColor = sentByMe ? Kirigami.Theme.backgroundColor : Kirigami.Theme.highlightColor
					var textColor = sentByMe ? Kirigami.Theme.textColor : Kirigami.Theme.buttonTextColor
					return Qt.tint(textColor, Qt.rgba(bgColor.r, bgColor.g, bgColor.b, 0.7))
				}
			}

			ColumnLayout {
				visible: isSpoiler && isShowingSpoiler || !isSpoiler


				Controls.ToolButton {
					visible: {
						mediaType !== Enums.MessageText &&
								!isLoading &&
								mediaLocation === "" &&
								mediaGetUrl !== ""
					}
					text: qsTr("Download")
					onClicked: {
						print("Downloading " + mediaGetUrl + "...")
						kaidan.downloadMedia(msgId, mediaGetUrl)
					}
				}

				// media loader
				Loader {
					id: media
					source: {
						if (mediaType === Enums.MessageImage &&
							mediaLocation !== "")
							"ChatMessageImage.qml"
						else
							""
					}
					property string sourceUrl: "file://" + mediaLocation
					Layout.maximumWidth: root.width - Kirigami.Units.gridUnit * 6
					Layout.preferredHeight: loaded ? item.paintedHeight : 0
				}


				// message body
				Controls.Label {
					id: messageLabel
					visible: messageBody !== ""
					text: kaidan.utils.formatMessage(messageBody)
					textFormat: Text.StyledText
					wrapMode: Text.Wrap
					color: sentByMe ? Kirigami.Theme.buttonTextColor
									: Kirigami.Theme.complementaryTextColor
					onLinkActivated: Qt.openUrlExternally(link)

					Layout.maximumWidth: mediaType === Enums.MessageImage && media.width !== 0
										? media.width
										: root.width - Kirigami.Units.gridUnit * 6
				}
				Kirigami.Separator {
					visible: isSpoiler && isShowingSpoiler
					Layout.fillWidth: true
					color: {
						var bgColor = sentByMe ? Kirigami.Theme.backgroundColor : Kirigami.Theme.highlightColor
						var textColor = sentByMe ? Kirigami.Theme.textColor : Kirigami.Theme.buttonTextColor
						return Qt.tint(textColor, Qt.rgba(bgColor.r, bgColor.g, bgColor.b, 0.7))
					}
				}
			}
			// message meta: date, isRead
			RowLayout {
				// progress bar for upload/download status
				Controls.ProgressBar {
					visible: isLoading
					value: isLoading ? upload.progress : 0
				}

				Controls.Label {
					id: dateLabel
					text: Qt.formatDateTime(dateTime, "dd. MMM yyyy, hh:mm")
					color: sentByMe ? Kirigami.Theme.disabledTextColor
					                : Qt.darker(Kirigami.Theme.disabledTextColor, 1.3)
					font.pixelSize: Kirigami.Units.gridUnit * 0.8
				}

				Image {
					id: checkmark
					visible: (sentByMe && isRead)
					source: kaidan.utils.getResourcePath("images/message_checkmark.svg")
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
