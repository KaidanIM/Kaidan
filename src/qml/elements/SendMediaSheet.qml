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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

Kirigami.OverlaySheet {
	id: root

	property string targetJid
	property url source
	property int sourceType

	showCloseButton: false

	contentItem: ColumnLayout {
		// message type preview
		MediaPreviewLoader {
			id: mediaLoader

			mediaSheet: root
			mediaSource: root.source
			mediaSourceType: root.sourceType
		}

		// TODO: - Maybe add option to change file name
		//       - Enabled/Disable image compression

		// caption/description text field
		// disabled for now; most other clients (currently) don't support this
		Controls.TextField {
			id: descField

			visible: false
			placeholderText: qsTr("Caption")
			selectByMouse: true

			Layout.fillWidth: true
			Layout.topMargin: Kirigami.Units.largeSpacing
		}

		// buttons for send/cancel
		RowLayout {
			Layout.topMargin: Kirigami.Units.largeSpacing
			Layout.fillWidth: true

			Controls.Button {
				text: qsTr("Cancel")

				Layout.fillWidth: true

				onClicked: close()
			}

			Controls.Button {
				id: sendButton

				text: qsTr("Send")

				Layout.fillWidth: true

				onClicked: {
					switch (root.sourceType) {
					case Enums.MessageType.MessageUnknown:
					case Enums.MessageType.MessageText:
					case Enums.MessageType.MessageGeoLocation:
						break
					case Enums.MessageType.MessageImage:
					case Enums.MessageType.MessageAudio:
					case Enums.MessageType.MessageVideo:
					case Enums.MessageType.MessageFile:
					case Enums.MessageType.MessageDocument:
						kaidan.sendFile(root.targetJid, root.source, descField.text)
						break
					}

					close()
				}
			}
		}

		Keys.onPressed: {
			if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
				sendButton.clicked()
			}
		}
	}

	onSheetOpenChanged: {
		if (!sheetOpen) {
			targetJid = ''
			source = ''
			sourceType = Enums.MessageType.MessageUnknown
			descField.clear()
		}
	}

	function sendMessageType(jid, type) {
		targetJid = jid
		sourceType = type
		open()
	}

	function sendFile(jid, url) {
		source = url
		sendMessageType(jid, MediaUtilsInstance.messageType(url))
	}
}
