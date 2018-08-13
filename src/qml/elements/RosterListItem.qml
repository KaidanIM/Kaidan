/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017-2018 Kaidan developers and contributors
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

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.0 as Kirigami

Kirigami.SwipeListItem {
	id: listItem

	property string name
	property string jid
	property string lastMessage
	property int unreadMessages
	property string avatarImagePath
	property int presenceType
	property string statusMsg

	topPadding: Kirigami.Units.smallSpacing * 1.5
	height: Kirigami.Units.gridUnit * 3.5

	RowLayout {
		spacing: Kirigami.Units.gridUnit * 0.5

		// left side: Avatar
		Item {
			id: avatarSpace
			Layout.preferredHeight: parent.height
			Layout.preferredWidth: parent.height

			RoundImage {
				id: avatar
				anchors.fill: parent
				source: avatarImagePath
				width: height
				fillMode: Image.PreserveAspectFit
				mipmap: true

				Controls.ToolTip {
					visible: hovered
					delay: Qt.styleHints.mousePressAndHoldInterval
					text: generateToolTipText(listItem.name, listItem.jid,
					                          listItem.presenceType, listItem.statusMsg)
				}
			}

			Rectangle {
				id: presenceIndicator
				visible: presenceType !== 8 // invisible when presence is invalid
				anchors.right: avatarSpace.right
				anchors.bottom: avatarSpace.bottom

				width: Kirigami.Units.gridUnit
				height: Kirigami.Units.gridUnit

				color: presenceTypeToColor(presenceType)
				radius: Math.min(width, height) * 0.5
				// shadow
				layer.enabled: presenceIndicator.visible
				layer.effect: DropShadow {
					verticalOffset: Kirigami.Units.gridUnit * 0.08
					horizontalOffset: Kirigami.Units.gridUnit * 0.08
					color: Kirigami.Theme.disabledTextColor
					samples: 10
					spread: 0.1
				}
			}
		}

		// right side
		ColumnLayout {
			spacing: Kirigami.Units.smallSpacing
			Layout.fillWidth: true

			// contact name
			Kirigami.Heading {
				text: name
				textFormat: Text.PlainText
				elide: Text.ElideRight
				maximumLineCount: 1
				level: 3
				Layout.fillWidth: true
				Layout.maximumHeight: Kirigami.Units.gridUnit * 1.5
			}
			// bottom
			RowLayout {
				Layout.fillWidth: true

				Controls.Label {
					Layout.fillWidth: true
					elide: Text.ElideRight
					maximumLineCount: 1
					text: {
						presenceType === 7 ? // error presence type
						qsTr("Error: Please check the JID.") :
						kaidan.removeNewLinesFromString(lastMessage)
					}
					textFormat: Text.PlainText
					font.pixelSize: 16
				}
			}
		}

		// unread message counter
		MessageCounter {
			visible: unreadMessages > 0
			counter: unreadMessages

			Layout.preferredHeight: Kirigami.Units.gridUnit * 1.25
			Layout.preferredWidth: Kirigami.Units.gridUnit * 1.25
		}
	}

	/**
	 * Returns the colour beloning to the given presence status type
	 */
	function presenceTypeToColor(type) {
		return type === 0 ? "green" :        // available
			type === 1 ? "darkgreen" :     // chat
			type === 2 ? "orange" :        // away
			type === 3 ? "orange" :        // do not disturb
			type === 4 ? "orange" :        // extended away
			type === 7 ? "red" :           // error
			type === 6 ? "red" :           // error
			"lightgrey" // unavailable (offline) (5), probe (6), invalid (8)
	}

	/**
	 * Generates a styled text telling some basic information about the contact,
	 * is used for a tooltip
	 */
	function generateToolTipText(name, jid, statusType, statusMsg) {
		// header (contact name)
		var string = "<h3>" + name + "</h3>"
		// in small: JID (only if differs name)
		if (name !== jid) {
			string += "<h5><i>" + jid + "</i></h5>"
		}

		// presence status type
		string += "<font color='" + presenceTypeToColor(statusType) + "'>"
		string += statusType === 0 ? qsTr("Available") :
		          statusType === 1 ? qsTr("Free for chat") :
		          statusType === 2 ? qsTr("Away") :
		          statusType === 3 ? qsTr("Do not disturb") :
		          statusType === 4 ? qsTr("Away for longer") :
		          statusType === 5 ? qsTr("Offline") :
		          statusType === 7 ? qsTr("Error") :
		          qsTr("Invalid")
		string += "</font>"

		// presence status message
		if (statusMsg !== "") {
			string += ": " + statusMsg
		}
		return string
	}
}
