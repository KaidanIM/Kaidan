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
import org.kde.kirigami 2.8 as Kirigami

Controls.Dialog {
	id: aboutDialog
	modal: true
	standardButtons: Controls.Dialog.Ok
	onAccepted: close()

	GridLayout {
		anchors.fill: parent
		flow: root.width > root.height ? GridLayout.LeftToRight : GridLayout.TopToBottom
		columnSpacing: 20
		rowSpacing: 20

		Image {
			source: kaidan.utils.getResourcePath("images/kaidan.svg")
			Layout.preferredWidth: Kirigami.Units.gridUnit * 9
			Layout.preferredHeight: Kirigami.Units.gridUnit * 9
			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.alignment: Qt.AlignCenter
			fillMode: Image.PreserveAspectFit
			mipmap: true
			sourceSize: Qt.size(width, height)
		}

		ColumnLayout {
			Layout.fillWidth: true
			Layout.fillHeight: true
			spacing: Kirigami.gridUnit * 0.6

			Kirigami.Heading {
				text: "Kaidan " + kaidan.utils.getVersionString()
				textFormat: Text.PlainText
				wrapMode: Text.WordWrap
				Layout.fillWidth: true
				horizontalAlignment: Qt.AlignHCenter
			}

			Controls.Label {
				text: "<i>" + qsTr("A simple, user-friendly Jabber/XMPP client") + "</i>"
				textFormat: Text.StyledText
				wrapMode: Text.WordWrap
				Layout.fillWidth: true
				horizontalAlignment: Qt.AlignHCenter
			}

			Controls.Label {
				text: "<b>" + qsTr("License:") + "</b> GPLv3+ / CC BY-SA 4.0"
				textFormat: Text.StyledText
				wrapMode: Text.WordWrap
				Layout.fillWidth: true
				horizontalAlignment: Qt.AlignHCenter
			}

			Controls.Label {
				text: "Copyright © 2016-2019\nKaidan developers and contributors"
				textFormat: Text.PlainText
				wrapMode: Text.WordWrap
				Layout.fillWidth: true
				Layout.preferredWidth: contentWidth
				horizontalAlignment: Qt.AlignHCenter
			}

			Controls.ToolButton {
				text: qsTr("View source code online")
				onClicked: Qt.openUrlExternally("https://invent.kde.org/kde/kaidan")
				Layout.fillWidth: true
				Layout.alignment: Qt.AlignHCenter
			}
		}
	}
}
