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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Controls.Material 2.12 as Material
import QtGraphicalEffects 1.12
import QtMultimedia 5.12
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0

import "elements"

/**
 * This page is used for logging in by scanning a QR code which contains an XMPP login URI.
 */
Kirigami.Page {
	id: root
	title: qsTr("Scan QR code")
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0

	property bool cameraEnabled: false
	property bool loggingIn: false

	// hint for camera issues
	Kirigami.InlineMessage {
		visible: cameraEnabled && text !== ""
		anchors.centerIn: parent
		width: 300
		height: 60
		text: {
			switch (camera.availability) {
			case Camera.Unavailable:
			case Camera.ResourceMissing:
				// message to be shown if no camera can be found
				return qsTr("There is no camera available.")
			case Camera.Busy:
				// message to be shown if the found camera is not usable
				return qsTr("Your camera is busy.\nTry to close other applications using the camera.")
			default:
				// no message if no issue could be found
				return ""
			}
		}
	}

	// camera with continuous focus in the center of the video
	Camera {
		id: camera
		focus.focusMode: Camera.FocusContinuous
		focus.focusPointMode: Camera.FocusPointCenter

		// Show camera output if this page is visible and the camera enabled.
		cameraState: {
			if (visible && cameraEnabled)
				return Camera.ActiveState
			return Camera.LoadedState
		}

		Component.onCompleted: {
			scannerFilter.setCameraDefaultVideoFormat(camera);
		}
	}

	// filter which converts the video frames to images and decodes a containing QR code
	QrCodeScannerFilter {
		id: scannerFilter

		onScanningSucceeded: {
			if (!loggingIn) {
				// Login by the data from the decoded QR code.
				loggingIn = Kaidan.logInByUri(result)
			}
		}

		onUnsupportedFormatReceived: {
			pageStack.layers.pop()
			passiveNotification(qsTr("The camera format '%1' is not supported.").arg(format))
		}
	}

	// video output from the camera which is shown on the screen and decoded by a filter
	VideoOutput {
		anchors.fill: parent
		fillMode: VideoOutput.PreserveAspectCrop
		source: camera
		autoOrientation: true
		filters: [scannerFilter]
	}

	ColumnLayout {
		id: loadingArea
		z: 2
		anchors.centerIn: parent
		visible: loggingIn

		Controls.BusyIndicator {
			Layout.alignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "<i>" + qsTr("Logging in…") + "</i>"
			color: Kirigami.Theme.textColor
		}

		Connections {
			target: Kaidan

			onConnectionStateChanged: {
				if (loggingIn) {
					switch (Kaidan.connectionState) {
					case Enums.StateConnected:
						popAllLayers()
						break
					case Enums.StateDisconnected:
						showPassiveNotificationForConnectionError()
						loggingIn = false
						break
					}
				}
			}
		}
	}

	Rectangle {
		z: 1
		anchors.fill: loadingArea
		anchors.margins: -8
		radius: 3
		opacity: 0.90
		color: Kirigami.Theme.backgroundColor

		visible: loadingArea.visible
	}

	ColumnLayout {
		id: explanationArea
		z: 1
		anchors.margins: 18
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		CenteredAdaptiveText {
			id: explanation
			text: qsTr("Scan the QR code from your existing device to transfer your account.")
			width: parent.width
			Layout.topMargin: 10
			scaleFactor: 1.5
		}

		Image {
			source: Utils.getResourcePath("images/onboarding/account-transfer.svg")
			sourceSize.height: parent.height
			visible: explanation.visible
			fillMode: Image.PreserveAspectFit
			Layout.fillHeight: true
			Layout.fillWidth: true
		}

		// placeholder for the explanation text and image when they are invisible
		Item {
			Layout.fillHeight: true
		}

		ColumnLayout {
			Layout.alignment: Qt.AlignHCenter
			Layout.maximumWidth: largeButtonWidth

			// button for showing or hiding the explanation
			CenteredAdaptiveHighlightedButton {
				label.text: checked ? qsTr("Show explanation") : qsTr("Scan")
				checkable: true

				onClicked: {
					if (!cameraEnabled) {
						camera.start()
						cameraEnabled = true
					}
					explanation.visible = !explanation.visible
				}
			}

			// button for skipping the scanning
			CenteredAdaptiveButton {
				label.text: qsTr("Continue without QR code")
				onClicked: pageStack.layers.push(registrationLoginDecisionPage)
			}
		}
	}

	// background of the explanation area
	Rectangle {
		anchors.fill: explanationArea
		anchors.margins: -8
		visible: explanation.visible
		radius: roundedCornersRadius
		opacity: 0.90
		color: Kirigami.Theme.backgroundColor
	}
}
