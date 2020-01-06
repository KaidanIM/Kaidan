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
 * This element is used in the @see SendMediaSheet to share information about a location to
 * the user. It just displays the map in a rectangle.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import QtPositioning 5.2 as Positioning
import QtLocation 5.3 as Location
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

MediaPreview {
	id: root

	property var selectedGeoLocation

	// Quicksy / Conversations compatible
	mediaSource: selectedGeoLocation !== undefined && selectedGeoLocation.isValid
				 ? 'geo:%1,%2'.arg(selectedGeoLocation.latitude.toString()).arg(selectedGeoLocation.longitude.toString())
				 : ''

	Layout.preferredHeight: message ? messageSize : Kirigami.Units.gridUnit * 18
	Layout.preferredWidth: Kirigami.Units.gridUnit * 32
	Layout.maximumWidth: message ? messageSize : -1

	ColumnLayout {
		anchors {
			fill: parent
		}

		Location.Map {
			id: map

			zoomLevel: (maximumZoomLevel - minimumZoomLevel) / 2
			center: root.selectedGeoLocation

			plugin: Location.Plugin {
				preferred: ["osm", "mapboxgl"]
			}

			gesture {
				flickDeceleration: 3000
				enabled: !root.message
			}

			Layout.fillHeight: true
			Layout.fillWidth: true

			Keys.onPressed: {
				if (!root.mediaSource) {
					if (event.key === Qt.Key_Plus) {
						map.zoomLevel++
					} else if (event.key === Qt.Key_Minus) {
						map.zoomLevel--
					}
				}
			}

			onCenterChanged: {
				if (!followMe.checked && !root.message) {
					root.selectedGeoLocation = center
				}
			}

			onCopyrightLinkActivated: Qt.openUrlExternally(link)

			onErrorChanged: {
				if (map.error !== Location.Map.NoError) {
					console.log("***", map.errorString)
				}
			}

			Positioning.PositionSource {
				id: currentPosition

				active: !root.message

				onPositionChanged: {
					if (position.coordinate.isValid) {
						if (!root.message && followMe.checked) {
							root.selectedGeoLocation = position.coordinate
							map.center = root.selectedGeoLocation
						}
					} else {
						console.log('***', 'Can not locate this device.')
					}
				}

				onSourceErrorChanged: {
					if (sourceError !== Positioning.PositionSource.NoError) {
						console.log("***", sourceError)
						stop()
					}
				}

				onUpdateTimeout: {
					console.log("***", "Position lookup timeout.")
				}
			}

			Location.MapQuickItem {
				id: currentPositionMarker

				visible: !root.message
				coordinate: currentPosition.position.coordinate
				anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)

				sourceItem: Rectangle {
					height: 12
					width: height
					color: "#0080FF"
					radius: height / 2

					border {
						width: 2
						color: Qt.lighter(color)
					}
				}
			}

			Location.MapQuickItem {
				id: userPositionMarker

				coordinate: followMe.checked ? root.selectedGeoLocation : map.center
				anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height)

				sourceItem: Kirigami.Icon {
					source: MediaUtilsInstance.newMediaIconName(Enums.MessageType.MessageGeoLocation)
					height: 48
					width: height
					color: "#e41e25"
					smooth: true
				}
			}

			Controls.ToolButton {
				id: followMe

				visible: !root.message
				enabled: currentPosition.supportedPositioningMethods !== Positioning.PositionSource.NoPositioningMethods
				checkable: true
				checked: true

				icon {
					name: checked
						  ? MediaUtilsInstance.newMediaIconName(Enums.MessageType.MessageGeoLocation)
						  : 'crosshairs'
				}

				anchors {
					right: parent.right
					bottom: parent.bottom
					margins: Kirigami.Units.gridUnit
				}

				background: Rectangle {
					radius: height / 2
					color: "green"

					border {
						color: 'white'
						width: 2
					}
				}

				contentItem: Kirigami.Icon {
					source: parent.icon.name

					anchors {
						fill: parent
						margins: Kirigami.Units.smallSpacing
					}
				}

				onCheckedChanged: {
					if (checked) {
						root.selectedGeoLocation = currentPosition.position.coordinate
						map.center = root.selectedGeoLocation
					} else {
						root.selectedGeoLocation = map.center
					}
				}
			}

			Controls.Slider {
				id: zoomSlider

				visible: !root.message
				from: map.minimumZoomLevel
				to: map.maximumZoomLevel
				orientation : Qt.Vertical
				value: map.zoomLevel
				width: Kirigami.Units.gridUnit * 1.4
				z: map.z + 3

				anchors {
					left: parent.left
					top: parent.top
					bottom: parent.bottom
					margins: Kirigami.Units.gridUnit
				}

				onValueChanged: {
					map.zoomLevel = value
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
											 .arg(root.selectedGeoLocation.latitude).arg(root.selectedGeoLocation.longitude))
					}
				}
			}
		}
	}
}
