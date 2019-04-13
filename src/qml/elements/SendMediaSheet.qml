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
import org.kde.kirigami 2.0 as Kirigami

Kirigami.OverlaySheet {
	id: root

	property string jid
	property string fileUrl
	property bool isImage: kaidan.utils.isImageFile(fileUrl)

	showCloseButton: false

	ColumnLayout {
		// isImage ? image preview inside a grey box
		//         : fallback file info
		Loader {
			Layout.preferredWidth: isImage ? Kirigami.Units.gridUnit * 32
			                               : Kirigami.Units.gridUnit * 20
			Layout.preferredHeight: isImage ? Kirigami.Units.gridUnit * 18
			                                : Kirigami.Units.gridUnit * 3.85
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter

			property string sourceUrl: fileUrl

			source: isImage ? "MediaPreviewImage.qml" : "MediaPreviewOther.qml"
		}

		// TODO: - Maybe add option to change file name
		//       - Enabled/Disable image compression

		// caption/description text field
		// disabled for now; most other clients (currently) don't support this
		Controls.TextField {
			id: descField
			visible: false
			text: ""
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
				onClicked: {
					close()
					descField.text = ""
				}
				Layout.fillWidth: true
			}

			Controls.Button {
				id: sendButton
				text: qsTr("Send")
				onClicked: {
					kaidan.sendFile(jid, fileUrl, descField.text)
					close()
					descField.text = ""
				}
				Layout.fillWidth: true
			}
		}

		Keys.onPressed: {
			if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
				sendButton.clicked()
			}
		}
	}
}
