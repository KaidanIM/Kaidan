/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 JBBgameich <jbb.mail@gmx.de>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 1.0 as Kirigami

Kirigami.ScrollablePage {
	id: aboutPage
	title: qsTr("About")

	Column {
		width: parent.width

		Image {
			source: kaidan.getResourcePath("images/kaidan.svg")
			height: aboutPage.height * 0.5
			fillMode: Image.PreserveAspectFit
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Kirigami.Label {
			font.bold: true
			text: "<h2>Kaidan " + kaidan.getVersionString() + "</h2>"
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Kirigami.Label {
			text: qsTr("A simple, user-friendly Jabber/XMPP client")
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Kirigami.Label {
			text: qsTr("License:") + " GPLv3+ / CC BY-SA 4.0"
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Kirigami.Label {
			text: "Copyright (C) 2016-2017 Kaidan developers and contributors"
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Controls.ToolButton {
			text: qsTr("Sourcecode on Github")
			onClicked: Qt.openUrlExternally("https://github.com/KaidanIM/Kaidan")
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Controls.Button {
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Close")
			onClicked: pageStack.pop()
		}
	}
}
