/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

import im.kaidan.kaidan 1.0

import "elements"
import "elements/fields"

Kirigami.Page {
	title: qsTr("Log in")

	contextualActions: [
		Kirigami.Action {
			text: qsTr("Log in using a QR-Code")
			icon.name: "view-barcode"
			onTriggered: pageStack.layers.push(qrCodeScannerPage)
		}
	]

	ColumnLayout {
		anchors.fill: parent

		Kirigami.Heading {
			text: qsTr("Log in to your XMPP account")
			wrapMode: Text.WordWrap
			Layout.fillWidth: true
			horizontalAlignment: Qt.AlignHCenter
		}

		ColumnLayout {
			width: parent.width
			Layout.fillWidth: true

			// For desktop or tablet devices
			Layout.alignment: Qt.AlignCenter
			Layout.maximumWidth: Kirigami.Units.gridUnit * 25

			// JID field
			JidField {
				id: jidField

				// Simulate the pressing of the connect button.
				inputField {
					onAccepted: connectButton.clicked()
				}
			}

			// password field
			PasswordField {
				id: passwordField

				// Simulate the pressing of the connect button.
				inputField {
					onAccepted: connectButton.clicked()
				}
			}

			// Connect button
			CenteredAdaptiveHighlightedButton {
				id: connectButton
				label.text: qsTr("Connect")

				state: kaidan.connectionState !== Enums.StateDisconnected ? "connecting" : ""
				states: [
					State {
						name: "connecting"
						PropertyChanges {
							target: connectButton
							label.text: "<i>" + qsTr("Connecting…") + "</i>"
							label.color: "black"
							label.textFormat: Text.StyledText
							enabled: false
						}
					}
				]

				// Connect to the server and authenticate by the entered credentials if the JID is valid and a password entered.
				onClicked: {
					// If the JID is invalid, focus its field.
					if (!jidField.valid) {
						jidField.forceFocus()
					// If the password is invalid, focus its field.
					// This also implies that if the JID field is focused and the password invalid, the password field will be focused instead of immediately trying to connect.
					} else if (!passwordField.valid) {
						passwordField.forceFocus()
					} else {
						kaidan.jid = jidField.text
						kaidan.password = passwordField.text
						kaidan.mainConnect()
					}
				}
			}
		}

		// placeholder
		Item {
			Layout.preferredHeight: Kirigami.Units.gridUnit * 3
		}
	}

	Component.onCompleted: {
		jidField.forceFocus()
	}

	Connections {
		target: kaidan

		onConnectionErrorChanged: showPassiveNotificationForConnectionError()
	}
}
