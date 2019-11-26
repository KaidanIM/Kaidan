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
import QtQuick.Controls 2.3 as Controls
import QtMultimedia 5.8
import org.kde.kirigami 2.8 as Kirigami
import im.kaidan.kaidan 1.0

// QR code scanner output and decoding for logging in by a decoded XMPP URI
Kirigami.Page {
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0

	title: qsTr("Scan QR code")

	// message to be shown if no camera can be found
	Kirigami.InlineMessage {
		visible: {
			camera.availability === Camera.Unavailable ||
			camera.availability === Camera.ResourceMissing
		}
		anchors.centerIn: parent
		width: 300
		height: 60
		text: qsTr("There is no camera available.")
	}

	// message to be shown if the found camera is not usable
	Kirigami.InlineMessage {
		visible: camera.availability === Camera.Busy
		anchors.centerIn: parent
		width: 300
		height: 60
		text: qsTr("Your camera is busy.\nTry to close other applications using the camera.")
	}

	// video output from the camera which is shown on the screen and decoded by a filter
	VideoOutput {
		id: viewfinder
		anchors.fill: parent
		fillMode: VideoOutput.PreserveAspectCrop
		source: camera
		autoOrientation: true
		filters: [scannerFilter]
	}

	// filter which converst the video frames to images and decodes a containing QR code
	QrCodeScannerFilter {
		id: scannerFilter
		onScanningSucceeded: {
			pageStack.layers.pop()
			// login by the data from the decoded QR code
			kaidan.loginByUri(result)
		}
		onUnsupportedFormatReceived: {
			pageStack.layers.pop()
			passiveNotification(qsTr("The camera format '%1' is not supported.").arg(format))
		}
	}

	// camera with continuous focus in the center of the video
	Camera {
		id: camera
		focus {
			focusMode: Camera.FocusContinuous
			focusPointMode: Camera.FocusPointCenter
		}

		Component.onCompleted: {
			scannerFilter.setCameraDefaultVideoFormat(camera);
		}
	}
}
