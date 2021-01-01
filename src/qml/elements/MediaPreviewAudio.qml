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

/**
 * This element is used in the @see SendMediaSheet to display information about a selected audio file to
 * the user. It just displays the audio in a rectangle.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtMultimedia 5.12 as Multimedia
import org.kde.kirigami 2.12 as Kirigami

import MediaUtils 0.1

MediaPreview {
	id: root

	property alias placeHolder: placeHolder.data

	readonly property alias player: mediaPlayer
	readonly property alias playPauseButton: playPause

	color: 'transparent'

	Layout.preferredHeight: message ? Kirigami.Units.gridUnit * 2.4 : Kirigami.Units.gridUnit * 2.45
	Layout.preferredWidth: message ? Kirigami.Units.gridUnit * 10 : Kirigami.Units.gridUnit * 20
	Layout.maximumWidth: message ? messageSize : -1

	Multimedia.MediaPlayer {
		id: mediaPlayer

		source: root.mediaSource
		volume: volumePlayer.volume

		onStopped: seek(0)
	}

	ColumnLayout {
		anchors {
			fill: parent
		}

		Item {
			id: placeHolder

			visible: children.length > 0

			Layout.fillHeight: true
			Layout.fillWidth: true
		}

		RowLayout {
			Layout.topMargin: 6
			Layout.margins: 10
			Layout.fillWidth: true

			Controls.ToolButton {
				id: playPause

				icon.name: mediaPlayer.playbackState === Multimedia.MediaPlayer.PlayingState
						   ? 'media-playback-pause'
						   : 'media-playback-start'

				onClicked: {
					switch (mediaPlayer.playbackState) {
					case Multimedia.MediaPlayer.PlayingState:
						mediaPlayer.pause()
						break
					case Multimedia.MediaPlayer.PausedState:
					case Multimedia.MediaPlayer.StoppedState:
						mediaPlayer.play()
						break
					}
				}
			}

			Controls.Slider {
				from: 0
				to: mediaPlayer.duration
				value: mediaPlayer.position
				enabled: mediaPlayer.seekable

				Layout.fillWidth: true

				Row {
					anchors {
						right: parent.right
						top: parent.top
						topMargin: -(parent.height / 2)
					}

					readonly property real fontSize: 7

					Controls.Label {
						text: MediaUtilsInstance.prettyDuration(mediaPlayer.position, mediaPlayer.duration)
						font.pointSize: parent.fontSize
						visible: mediaPlayer.duration > 0 && mediaPlayer.playbackState !== Multimedia.MediaPlayer.StoppedState
					}
					Controls.Label {
						text: ' / '
						font.pointSize: parent.fontSize
						visible: mediaPlayer.duration > 0 && mediaPlayer.playbackState !== Multimedia.MediaPlayer.StoppedState
					}
					Controls.Label {
						text: MediaUtilsInstance.prettyDuration(mediaPlayer.duration)
						visible: mediaPlayer.duration > 0
						font.pointSize: parent.fontSize
					}
				}

				onMoved: mediaPlayer.seek(value)
			}

			Controls.Slider {
				id: volumePlayer

				readonly property real volume: Multimedia.QtMultimedia.convertVolume(value,
																					 Multimedia.QtMultimedia.LogarithmicVolumeScale,
																					 Multimedia.QtMultimedia.LinearVolumeScale)

				from: 0
				to: 1.0
				value: 1.0
				visible: !root.message && !Kirigami.Settings.isMobile

				Layout.preferredWidth: root.width / 6
			}

			Controls.ToolButton {
				visible: root.showOpenButton

				icon {
					name: 'document-open'
				}

				onClicked: Qt.openUrlExternally(root.mediaSource)
			}
		}
	}
}

