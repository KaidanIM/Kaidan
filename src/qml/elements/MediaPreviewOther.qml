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
 * This element is used in the @see SendMediaSheet to display information about a selected file to
 * the user. It shows the file name, file size and a little file icon.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtGraphicalEffects 1.12
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

MediaPreview {
	id: root
	Layout.preferredHeight: Kirigami.Units.gridUnit * 3.85
	Layout.preferredWidth: layout.implicitWidth + layout.anchors.margins * 2
	Layout.maximumWidth: message ? messageSize : -1
	color: "transparent"

	// rounded box
	Rectangle {
		id: box
		color: {
			if(openButton.containsMouse) {
				if (messageBubble)
					Qt.darker(messageBubble.color, 1.08)
				else
					Qt.darker(rightMessageBubbleColor, 1.08)
			} else {
				if (messageBubble)
					Qt.darker(messageBubble.color, 1.05)
				else
					Qt.darker(rightMessageBubbleColor, 1.05)
			}
		}

		radius: roundedCornersRadius

		anchors {
			fill: parent
		}

		// content
		RowLayout {
			id: layout
			spacing: Kirigami.Units.gridUnit * 0.4

			anchors {
				fill: parent
				margins: layout.spacing
			}

			// left: file icon
			Item {
				Layout.fillHeight: true
				Layout.preferredWidth: height

				Kirigami.Icon {
					source: MediaUtilsInstance.iconName(root.mediaSource)
					isMask: root.showOpenButton
							? !(openButton.pressed || openButton.containsMouse)
							: true
					smooth: true
					height: 24 // we always want the 24x24 icon
					width: height

					anchors {
						centerIn: parent
					}
				}
			}

			// right: file description
			ColumnLayout {
				Layout.fillHeight: true
				Layout.fillWidth: true
				spacing: 0

				// file name
				Controls.Label {
					id: fileNameLabel
					Layout.fillWidth: true
					text: Utils.fileNameFromUrl(root.mediaSource)
					textFormat: Text.PlainText
					elide: Text.ElideRight
					maximumLineCount: 1
				}

				// mime type
				Controls.Label {
					id: fileMimeTypeLabel
					Layout.fillWidth: true
					text: MediaUtilsInstance.mimeTypeName(root.mediaSource)
					textFormat: Text.PlainText
					elide: Text.ElideRight
					maximumLineCount: 1
				}

				// file size
				Controls.Label {
					id: fileSizeLabel
					Layout.fillWidth: true
					text: Utils.fileSizeFromUrl(root.mediaSource)
					textFormat: Text.PlainText
					elide: Text.ElideRight
					maximumLineCount: 1
				}
			}
		}
	}

	MouseArea {
		id: openButton
		enabled: root.showOpenButton
		hoverEnabled: true

		anchors {
			fill: parent
		}

		onClicked: Qt.openUrlExternally(root.mediaSource)
	}

	Controls.ToolTip {
		delay: Kirigami.Units.longDuration
		parent: openButton
		text: root.mediaSource
	}
}
