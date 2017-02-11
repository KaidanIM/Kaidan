/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
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

import QtQuick 2.0
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 1.0 as Kirigami

Kirigami.ScrollablePage {
	property bool isRetry

	title: "Login"

	GridLayout {
		columns: 2
		anchors.fill: parent
		Layout.fillWidth: true

		// JID field
		Kirigami.Label {
			text: qsTr("Your Jabber-ID:")
		}
		Controls.TextField {
			id: jidField
			text: kaidan.jid
			placeholderText: qsTr("user@example.org")
			Layout.fillWidth: true
		}

		// Password field
		Kirigami.Label {
			text: qsTr("Your Password:")
		}
		Controls.TextField {
			id: passField
			text: kaidan.password
			placeholderText: qsTr("Password")
			echoMode: TextInput.Password
			Layout.fillWidth: true
		}

		// Connect button
		Controls.Button {
			id: connectButton
			text: isRetry ? qsTr("Retry") : qsTr("Connect")
			Layout.columnSpan: 2
			Layout.alignment: Qt.AlignRight
			onClicked: {
				// disable the button
				connectButton.enabled = false;
				// connect to given account data
				connectButton.text = "<i>" + qsTr("Connecting...") + "</i>";

				kaidan.jid = jidField.text;
				kaidan.password = passField.text;
				kaidan.mainConnect();
			}
		}
	}

	Component.onCompleted: {
		function goToRoster() {
			// we need to disconnect enableConnectButton to prevent calling it on normal disconnection
			kaidan.connectionStateDisconnected.disconnect(enableConnectButton);
			// open the roster page
			pageStack.replace(rosterPage);
		}

		function enableConnectButton() {
			isRetry = true;
			connectButton.text = qsTr("Retry");
			connectButton.enabled = true;
		}

		// connect functions to back-end events
		kaidan.connectionStateConnected.connect(goToRoster);
		kaidan.connectionStateDisconnected.connect(enableConnectButton);
	}
}
