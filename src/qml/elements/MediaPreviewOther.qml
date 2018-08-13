/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami

Item {
	id: fileInfo

	// rounded box
	Rectangle {
		id: box
		anchors.fill: fileInfo

		color: Kirigami.Theme.highlightColor
		radius: Kirigami.Units.smallSpacing * 2

		layer.enabled: box.visible
		layer.effect: DropShadow {
			anchors.fill: box
			verticalOffset: Kirigami.Units.gridUnit * 0.08
			horizontalOffset: Kirigami.Units.gridUnit * 0.08
			color: Kirigami.Theme.disabledTextColor
			samples: 10
			spread: 0.1
		}
	}

	// content
	RowLayout {
		// left: file icon
		Rectangle {
			Layout.preferredWidth: Kirigami.Units.gridUnit * 2.8
			Layout.preferredHeight: Kirigami.Units.gridUnit * 2.8
			Layout.margins: Kirigami.Units.gridUnit * 0.5

			radius: height * 0.5
			color: Kirigami.Theme.backgroundColor

			Kirigami.Icon {
				source: "text-x-plain"
				isMask: true
				smooth: true
				anchors.centerIn: parent
				width: 24 // we always want the 24x24 icon
				height: 24
			}
		}

		// right: file description
		ColumnLayout {
			Layout.preferredWidth: Kirigami.Units.gridUnit * 13.5

			// file name
			Controls.Label {
				Layout.fillWidth: true
				text: kaidan.fileNameFromUrl(sourceUrl)
				textFormat: Text.PlainText
				elide: Text.ElideRight
				maximumLineCount: 1
			}

			// file size
			Controls.Label {
				Layout.fillWidth: true
				text: kaidan.fileSizeFromUrl(sourceUrl)
				textFormat: Text.PlainText
				elide: Text.ElideRight
				maximumLineCount: 1
			}
		}
	}
}
