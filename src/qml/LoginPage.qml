/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.0 as Kirigami
import io.github.kaidanim 1.0

Kirigami.Page {
	title: qsTr("Log in")

	ColumnLayout {
		anchors.fill: parent

		Kirigami.Heading {
			text: qsTr("Log in to your XMPP account")
			wrapMode: Text.WordWrap
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		GridLayout {
			columns: 2
			width: parent.width
			Layout.fillWidth: true

			// JID field
			Controls.Label {
				id: jidLabel
				text: qsTr("Your Jabber-ID:")

				states: [
					State {
						name: "diaspora"
						PropertyChanges {
							target: jidLabel
							text: qsTr("Your diaspora*-ID:")
						}
					}
				]
			}
			Controls.TextField {
				id: jidField
				text: kaidan.jid
				placeholderText: qsTr("user@example.org")
				Layout.fillWidth: true
				selectByMouse: true

				states: [
					State {
						name: "diaspora"
						PropertyChanges {
							target: jidField
							placeholderText: qsTr("user@diaspora.pod")
						}
					}
				]
			}

			// Password field
			Controls.Label {
				text: qsTr("Your Password:")
			}
			Controls.TextField {
				id: passField
				text: kaidan.password
				echoMode: TextInput.Password
				selectByMouse: true
				Layout.fillWidth: true
			}

			// Connect button
			Controls.Button {
				id: connectButton
				Layout.columnSpan: 2
				Layout.alignment: Qt.AlignRight
				Layout.minimumWidth: connectLabel.width
				states: [
					State {
						name: "connecting"
						PropertyChanges {
							target: connectButton
							enabled: false
						}
						PropertyChanges {
							target: connectLabel
							text: "<i>" + qsTr("Connecting…") + "</i>"
						}
					}
				]

				onClicked: {
					// connect to given account data
					kaidan.jid = jidField.text
					kaidan.password = passField.text
					kaidan.mainConnect()
				}

				Controls.Label {
					id: connectLabel
					anchors.centerIn: connectButton
					text: qsTr("Connect")
					textFormat: Text.RichText
				}
			}

			// connect when return was pressed
			Keys.onPressed: {
				if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
					connectButton.clicked();
				}
			}
		}

		RowLayout {
			id: serviceBar
			anchors.horizontalCenter: parent.horizontalCenter

			Controls.ToolButton {
				Layout.preferredHeight: Kirigami.Units.gridUnit * 2.75
				Layout.preferredWidth: Kirigami.Units.gridUnit * 2.75
				onClicked: {
					jidField.state = "diaspora";
					jidLabel.state = "diaspora";
				}

				Image {
					source: kaidan.getResourcePath("images/diaspora.svg")
					fillMode: Image.PreserveAspectFit
					mipmap: true
					height: parent.height - Kirigami.Units.smallSpacing
				}
			}

			Controls.ToolButton {
				Layout.preferredHeight: Kirigami.Units.gridUnit * 2.75
				Layout.preferredWidth: Kirigami.Units.gridUnit * 2.75
				padding: Kirigami.Units.smallSpacing
				onClicked: {
					jidField.state = "";
					jidLabel.state = "";
				}

				Image {
					source: kaidan.getResourcePath("images/xmpp.svg")
					fillMode: Image.PreserveAspectFit
					mipmap: true
					height: parent.height - Kirigami.Units.smallSpacing
				}
			}
		}
	}

	function handleConnectionState(state) {
		if (state === Enums.StateConnecting) {
			connectButton.state = "connecting"
		} else {
			connectButton.state = ""
		}
	}

	Component.onCompleted: {
		kaidan.connectionStateChanged.connect(handleConnectionState)
	}

	Component.onDestruction: {
		kaidan.connectionStateChanged.disconnect(handleConnectionState)
	}
}
