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
 * This element is used in the @see SendMediaSheet to display information about a shared location to
 * the user. It just displays the map in a rectangle.
 */

import QtQuick 2.14
import QtQuick.Layouts 1.12
import QtLocation 5.12 as Location
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

MediaPreview {
	id: root

	Layout.preferredHeight: message ? messageSize : Kirigami.Units.gridUnit * 18
	Layout.preferredWidth: Kirigami.Units.gridUnit * 32
	Layout.maximumWidth: message ? messageSize : -1

	ColumnLayout {
		anchors {
			fill: parent
		}

		Location.Map {
			id: map

			zoomLevel: (maximumZoomLevel - minimumZoomLevel) / 1.2
			center: MediaUtilsInstance.locationCoordinate(root.mediaSource)
			copyrightsVisible: false

			plugin: Location.Plugin {
				preferred: ["osm", "mapboxgl"]
			}

			gesture {
				enabled: false
			}

			Layout.fillHeight: true
			Layout.fillWidth: true

			onErrorChanged: {
				if (map.error !== Location.Map.NoError) {
					console.log("***", map.errorString)
				}
			}

			Location.MapQuickItem {
				id: positionMarker

				coordinate: map.center
				anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height)

				sourceItem: Kirigami.Icon {
					source: MediaUtilsInstance.newMediaIconName(Enums.MessageType.MessageGeoLocation)
					height: 48
					width: height
					color: "#e41e25"
					smooth: true
				}
			}

			MouseArea {
				enabled: root.showOpenButton

				anchors {
					fill: parent
				}

				onClicked: {
					if (!Qt.openUrlExternally(root.message.messageBody)) {
						Qt.openUrlExternally('https://www.openstreetmap.org/?mlat=%1&mlon=%2&zoom=18&layers=M'
											 .arg(root.geoLocation.latitude).arg(root.geoLocation.longitude))
					}
				}
			}
		}
	}
}
