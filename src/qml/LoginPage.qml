/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
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

import QtQuick 2.0
import QtQuick.Controls 2.0 as Controls
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.0 as Kirigami

Kirigami.Page {
	property bool isRetry

	title: "Login"

	Column {
		width: parent.width
		spacing: 50

		Kirigami.Heading {
			text: qsTr("Log in to Kaidan")
			anchors.horizontalCenter: parent.horizontalCenter
		}

		GridLayout {
			columns: 2
			width: parent.width
			Layout.fillWidth: true

			// JID field
			Kirigami.Label {
				id: jidLabel
				text: qsTr("Your Jabber-ID:")
			}
			Controls.TextField {
				id: jidField
				text: kaidan.jid
				placeholderText: qsTr("user@example.org")
				Layout.fillWidth: true
				selectByMouse: true
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
				selectByMouse: true
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
					// indicate that we're connecting now
					connectButton.text = "<i>" + qsTr("Connecting...") + "</i>";

					// connect to given account data
					kaidan.jid = jidField.text;
					kaidan.password = passField.text;
					kaidan.mainConnect();
				}
			}
		}
		

		Controls.ToolBar {
			id: serviceBar
			height: Kirigami.Units.gridUnit * 3
			anchors.horizontalCenter: parent.horizontalCenter
			Material.foreground: "transparent"
			Material.background: "transparent"
			Material.accent: "transparent"

			Row {
				spacing: 20

				Controls.ToolButton {
					height: serviceBar.height
					onClicked: {
						jidField.placeholderText = qsTr("user@diaspora-pod.org");
						jidLabel.text = qsTr("Your Diaspora* ID");
					}

					Image { 
						source: kaidan.getResourcePath("images/diaspora.svg") 
						fillMode: Image.PreserveAspectFit
						height: serviceBar.height
					}
				}

				Controls.ToolButton {
					height: serviceBar.height
					onClicked: {
						jidField.placeholderText = qsTr("user@example.org");
						jidLabel.text = qsTr("Your Jabber ID");
					}

					Image { 
						source: kaidan.getResourcePath("images/xmpp.svg")
						fillMode: Image.PreserveAspectFit
						height: serviceBar.height
					}
				}
			}
		}
	}

	Component.onCompleted: {
		function openRosterPage() {
			// we need to disconnect enableConnectButton to prevent calling it on normal disconnection
			kaidan.connectionStateDisconnected.disconnect(enableConnectButton);

			// reenable the drawer
			globalDrawer.enabled = true;
			// open the roster page
			pageStack.replace(rosterPage);
		}

		function enableConnectButton() {
			isRetry = true;
			connectButton.text = qsTr("Retry");
			connectButton.enabled = true;
		}

		// connect functions to back-end events
		kaidan.connectionStateConnected.connect(openRosterPage);
		kaidan.connectionStateDisconnected.connect(enableConnectButton);

		// disable the drawer
		globalDrawer.enabled = false;
	}
}
