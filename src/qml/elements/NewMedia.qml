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

/**
 * This element is used in the @see SendMediaSheet to share information about a new media (picture, audio and video) to
 * the user. It just displays the camera image in a rectangle.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtMultimedia 5.12 as Multimedia
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

MediaPreview {
	id: root

	readonly property bool isNewImage: mediaSourceType === Enums.MessageType.MessageImage
	readonly property bool isNewAudio: mediaSourceType === Enums.MessageType.MessageAudio
	readonly property bool isNewVideo: mediaSourceType === Enums.MessageType.MessageVideo

	Layout.preferredHeight: Kirigami.Units.gridUnit * 14
	Layout.preferredWidth: Kirigami.Units.gridUnit * 14
	Layout.maximumWidth: -1

	MediaRecorder {
		id: recorder

		type: {
			switch (root.mediaSourceType) {
			case Enums.MessageType.MessageUnknown:
			case Enums.MessageType.MessageText:
			case Enums.MessageType.MessageFile:
			case Enums.MessageType.MessageDocument:
			case Enums.MessageType.MessageGeoLocation:
				return MediaRecorder.Type.Invalid
			case Enums.MessageType.MessageImage:
				return MediaRecorder.Type.Image
			case Enums.MessageType.MessageAudio:
				return MediaRecorder.Type.Audio
			case Enums.MessageType.MessageVideo:
				return MediaRecorder.Type.Video
			}
		}
	}

	ColumnLayout {
		anchors {
			fill: parent
		}

		Multimedia.VideoOutput {
			source: recorder
			focus : visible // to receive focus and capture key events when visible
			visible: !previewLoader.visible

			Layout.fillWidth: true
			Layout.fillHeight: true

			Controls.RoundButton {
				display: Controls.AbstractButton.IconOnly
				checkable: root.isNewVideo
				width: parent.width * .2
				height: width
				enabled: {
					return recorder.isReady ||
							(recorder.status >= MediaRecorder.Status.StartingStatus
							 && recorder.status <= MediaRecorder.Status.FinalizingStatus)
				}

				icon {
					width: parent.width
					height: width
					name: pressed || checked ? 'media-playback-stop' : 'media-record'
				}

				anchors {
					horizontalCenter: parent.horizontalCenter
					bottom: parent.bottom
					bottomMargin: -(height / 4)
				}

				onCheckedChanged: {
					if (checked) {
						recorder.record()
					} else {
						recorder.stop()
					}
				}

				onClicked: {
					if (root.isNewImage) {
						recorder.record()
					}
				}

				onPressAndHold: {
					if (root.isNewAudio) {
						recorder.record()
					}
				}

				onReleased: {
					if (root.isNewAudio) {
						recorder.stop()
					}
				}
			}

			Controls.Label {
				horizontalAlignment: Controls.Label.AlignRight
				text: {
					if (root.isNewImage) {
						return recorder.isReady ? qsTr('Ready') : qsTr('Initializing...')
					}

					switch (recorder.status) {
					case MediaRecorder.Status.UnavailableStatus:
						return qsTr('Unavailable')
					case MediaRecorder.Status.UnloadedStatus:
					case MediaRecorder.Status.LoadingStatus:
					case MediaRecorder.Status.LoadedStatus:
						return recorder.isReady ? qsTr('Ready') : qsTr('Initializing...')
					case MediaRecorder.Status.StartingStatus:
					case MediaRecorder.Status.RecordingStatus:
					case MediaRecorder.Status.FinalizingStatus:
						return qsTr('Recording... %1').arg(MediaUtilsInstance.prettyDuration(recorder.duration))
					case MediaRecorder.Status.PausedStatus:
						return qsTr('Paused %1').arg(MediaUtilsInstance.prettyDuration(recorder.duration))
					}
				}

				anchors {
					left: parent.left
					right: parent.right
					bottom: parent.bottom
				}
			}
		}

		MediaPreviewLoader {
			id: previewLoader

			mediaSource: recorder.actualLocation
			mediaSourceType: root.mediaSourceType
			showOpenButton: root.showOpenButton
			message: root.message
			mediaSheet: root.mediaSheet

			visible: mediaSource != '' && recorder.isReady

			Layout.fillHeight: true
			Layout.fillWidth: true

			RowLayout {
				visible: root.mediaSource == ''
				z: 1

				anchors {
					left: parent.left
					top: parent.top
					right: parent.right
				}

				Controls.ToolButton {
					icon.name: 'dialog-cancel'

					onClicked: {
						recorder.cancel()
					}
				}

				Item {
					Layout.fillWidth: true
				}

				Controls.ToolButton {
					icon.name: 'dialog-ok'

					onClicked: {
						root.mediaSource = previewLoader.mediaSource
					}
				}
			}
		}
	}

	Connections {
		target: root.mediaSheet
		enabled: target

		onRejected: {
			recorder.cancel()
		}
	}
}
