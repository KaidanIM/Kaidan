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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0

import "elements"

/**
 * This page is used for logging in by scanning a QR code which contains an XMPP login URI.
 */
ExplainedContentPage {
	title: qsTr("Scan QR code")
	explanationToggleButton.text: explanationToggleButton.checked ? qsTr("Show explanation") : qsTr("Scan QR code")

	explanationToggleButton.onClicked: {
		if (!scanner.cameraEnabled) {
			scanner.camera.start()
			scanner.cameraEnabled = true
		}
	}

	secondaryButton.text: qsTr("Continue without QR code")
	secondaryButton.onClicked: pageStack.layers.push(registrationLoginDecisionPage)

	property bool connecting: false

	QrCodeScanner {
		id: scanner
		anchors.fill: parent

		filter.onScanningSucceeded: {
			if (!connecting) {
				// Try to log in by the data from the decoded QR code.
				switch (Kaidan.logInByUri(result)) {
				case Enums.Connecting:
					connecting = true
					break;
				case Enums.PasswordNeeded:
					pageStack.layers.push(loginPage)
					break;
				case Enums.InvalidLoginUri:
				}
			}
		}
	}

	ColumnLayout {
		parent: explanation

		CenteredAdaptiveText {
			text: qsTr("Scan the QR code from your existing device to transfer your account.")
			Layout.topMargin: 10
			scaleFactor: 1.5
		}

		Image {
			source: Utils.getResourcePath("images/onboarding/account-transfer.svg")
			sourceSize.height: explanation.parent.height
			fillMode: Image.PreserveAspectFit
			Layout.fillHeight: true
			Layout.fillWidth: true
		}
	}

	Rectangle {
		z: loadingArea.z
		anchors.fill: loadingArea
		anchors.margins: -8
		radius: roundedCornersRadius
		color: Kirigami.Theme.backgroundColor
		opacity: 0.90
		visible: loadingArea.visible
	}

	ColumnLayout {
		id: loadingArea
		z: 2
		anchors.centerIn: parent
		visible: connecting

		Controls.BusyIndicator {
			Layout.alignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "<i>" + qsTr("Connecting…") + "</i>"
			color: Kirigami.Theme.textColor
		}

		Connections {
			target: Kaidan

			onConnectionStateChanged: {
				if (connecting) {
					switch (Kaidan.connectionState) {
					case Enums.StateConnected:
						popAllLayers()
						break
					case Enums.StateDisconnected:
						showPassiveNotificationForConnectionError()
						connecting = false
						break
					}
				}
			}
		}
	}
}