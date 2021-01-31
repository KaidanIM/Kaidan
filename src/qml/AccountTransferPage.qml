/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

import QtQuick 2.14
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0

import "elements"

/**
 * This page shows the user's credentials as a QR code or as plain text.
 *
 * It enables the user to log in on another device.
 */
ExplainedContentPage {
	title: qsTr("Transfer account to another device")

	primaryButton.text: primaryButton.checked ? qsTr("Hide QR code") : qsTr("Show as QR code")
	primaryButton.checkable: true
	primaryButton.onClicked: state = primaryButton.checked ? "qrCodeDisplayed" : "explanationDisplayed"

	secondaryButton.text: secondaryButton.checked ? qsTr("Hide text") : qsTr("Show as text")
	secondaryButton.checkable: true
	secondaryButton.onClicked: state = secondaryButton.checked ? "plainTextDisplayed" : "explanationDisplayed"

	state: "explanationDisplayed"

	states: [
		State {
			name: "explanationDisplayed"
			PropertyChanges { target: explanationArea; visible: true }
			PropertyChanges { target: primaryButton; checked: false }
			PropertyChanges { target: secondaryButton; checked: false }
			PropertyChanges { target: qrCode; visible: false }
			PropertyChanges { target: plainText; visible: false }
		},
		State {
			name: "qrCodeDisplayed"
			PropertyChanges { target: explanationArea; visible: false }
			PropertyChanges { target: primaryButton; checked: true }
			PropertyChanges { target: secondaryButton; checked: false }
			PropertyChanges { target: qrCode; visible: true }
			PropertyChanges { target: plainText; visible: false }
		},
		State {
			name: "plainTextDisplayed"
			PropertyChanges { target: explanationArea; visible: false }
			PropertyChanges { target: primaryButton; checked: false }
			PropertyChanges { target: secondaryButton; checked: true}
			PropertyChanges { target: qrCode; visible: false }
			PropertyChanges { target: plainText; visible: true }
		}
	]

	explanation: CenteredAdaptiveText {
		text: qsTr("Scan the QR code or enter the credentials as text on another device to log in on it.\n\nAttention:\nNever show this QR code to anyone else. It would allow unlimited access to your account!")
		verticalAlignment: Text.AlignVCenter
		Layout.fillHeight: true
		scaleFactor: 1.5
	}

	content: Item {
		Layout.fillHeight: true
		Layout.fillWidth: true

		QrCode {
			id: qrCode
			width: Math.min(largeButtonWidth, parent.width, parent.height)
			height: width
			anchors.centerIn: parent
		}

		Kirigami.FormLayout {
			id: plainText
			anchors.centerIn: parent

			RowLayout {
				Kirigami.FormData.label: qsTr("Chat address:")
				Layout.fillWidth: true

				Controls.Label {
					text: AccountManager.jid
					Layout.fillWidth: true
				}

				Controls.ToolButton {
					text: qsTr("Copy JID")
					icon.name: "edit-copy"
					display: Controls.AbstractButton.IconOnly
					flat: true
					Layout.alignment: Qt.AlignRight
					onClicked: Utils.copyToClipboard(AccountManager.jid)
				}
			}

			RowLayout {
				Kirigami.FormData.label: qsTr("Password:")
				visible: Kaidan.passwordVisibility === Kaidan.PasswordVisible
				Layout.fillWidth: true

				Controls.Label {
					text: AccountManager.password
					Layout.fillWidth: true
				}

				Controls.ToolButton {
					text: qsTr("Copy password")
					icon.name: "edit-copy"
					display: Controls.AbstractButton.IconOnly
					flat: true
					Layout.alignment: Qt.AlignRight
					onClicked: Utils.copyToClipboard(AccountManager.password)
				}
			}
		}
	}
}
