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
 * This element is used in the @see SendMediaSheet to display information about a selected file to
 * the user. It shows the file name, file size and a little file icon.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

MediaPreview {
	id: root

	color: 'transparent'

	Layout.preferredHeight: Kirigami.Units.gridUnit * 3.85
	Layout.preferredWidth: layout.implicitWidth + layout.anchors.margins * 2
	Layout.maximumWidth: message ? messageSize : -1

	// rounded box
	Rectangle {
		id: box

		visible: false
		color: root.message ? Qt.darker(Kirigami.Theme.buttonBackgroundColor, 1.2) : Kirigami.Theme.highlightColor
		radius: Kirigami.Units.smallSpacing * 2

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
			Rectangle {
				Layout.fillHeight: true
				Layout.preferredWidth: height

				radius: height * 0.5
				color: Kirigami.Theme.backgroundColor

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
				spacing: 0

				Layout.fillHeight: true
				Layout.fillWidth: true

				// file name
				Controls.Label {
					id: fileNameLabel
					Layout.fillWidth: true
					text: kaidan.utils.fileNameFromUrl(root.mediaSource)
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
					text: kaidan.utils.fileSizeFromUrl(root.mediaSource)
					textFormat: Text.PlainText
					elide: Text.ElideRight
					maximumLineCount: 1
				}
			}
		}
	}

	DropShadow {
		source: box
		verticalOffset: Kirigami.Units.gridUnit * 0.08
		horizontalOffset: Kirigami.Units.gridUnit * 0.08
		color: Kirigami.Theme.disabledTextColor
		samples: 10
		spread: 0.1

		anchors {
			fill: box
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
			text: '%1\n%2\n%3\n%4'
			      .arg(fileNameLabel.text)
			      .arg(fileMimeTypeLabel.text)
			      .arg(fileSizeLabel.text)
			      .arg(root.mediaSource)
		}
	}
}
