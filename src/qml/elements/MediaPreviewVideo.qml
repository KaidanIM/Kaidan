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

/**
 * This element is used in the @see SendMediaSheet to display information about a selected video to
 * the user. It just displays the video in a rectangle.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtMultimedia 5.8 as Multimedia
import org.kde.kirigami 2.8 as Kirigami

MediaPreviewAudio {
	id: root

	Layout.preferredHeight: message ? messageSize : Kirigami.Units.gridUnit * 18
	Layout.preferredWidth: Kirigami.Units.gridUnit * 32
	Layout.maximumWidth: message ? messageSize : -1

	placeHolder: Multimedia.VideoOutput {
		source: root.player
		fillMode: Image.PreserveAspectFit

		anchors {
			fill: parent
		}

		Kirigami.Icon {
			source: "video-x-generic"
			visible: root.player.playbackState === Multimedia.MediaPlayer.StoppedState

			width: parent.width
			height: parent.height

			anchors {
				centerIn: parent
			}
		}

		MouseArea {
			anchors {
				fill: parent
			}

			onPressed: root.playPauseButton.clicked()
		}
	}
}
