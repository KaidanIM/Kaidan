/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.1 as Controls
import org.kde.kirigami 2.0 as Kirigami

Kirigami.OverlaySheet {
	id: aboutSheet

	ColumnLayout {
		id: content
		spacing: Kirigami.Units.gridUnit * 0.6
		Layout.alignment: Qt.AlignHCenter

		Image {
			source: kaidan.getResourcePath("images/kaidan.svg")
			Layout.preferredHeight: Kirigami.Units.gridUnit * 15
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
			fillMode: Image.PreserveAspectFit
			mipmap: true
			sourceSize: Qt.size(width, height)

			Controls.ToolButton {
				anchors.right: parent.right
				text: "\u2715" // Unicode MULTIPLICATION X
				onClicked: aboutSheet.close()
			}
		}

		Kirigami.Heading {
			text: "Kaidan " + kaidan.getVersionString()
			textFormat: Text.PlainText
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "<i>" + qsTr("A simple, user-friendly Jabber/XMPP client") + "</i>"
			textFormat: Text.RichText
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "<b>" + qsTr("License:") + "</b> GPLv3+ / CC BY-SA 4.0"
			textFormat: Text.RichText
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "Copyright © 2016-2018\nKaidan developers and contributors"
			textFormat: Text.PlainText
			wrapMode: Text.WordWrap
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		Controls.ToolButton {
			text: qsTr("Source code on GitHub")
			onClicked: Qt.openUrlExternally("https://github.com/KaidanIM/Kaidan")
			Layout.fillWidth: true
			Layout.alignment: Qt.AlignHCenter
		}
	}
}
