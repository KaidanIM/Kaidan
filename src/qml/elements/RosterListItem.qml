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

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.0 as Kirigami
import im.kaidan.kaidan 1.0

Kirigami.SwipeListItem {
	id: listItem

	property string name
	property string jid
	property string lastMessage
	property int unreadMessages
	property string avatarImagePath
	property int presenceType
	property string statusMsg

	topPadding: 0
	leftPadding: 0
	bottomPadding: 0
	height: Kirigami.Units.gridUnit * 3.5

	RowLayout {
		spacing: Kirigami.Units.gridUnit * 0.5

		// left border: presence
		Rectangle {
			id: presenceIndicator
			visible: presenceType !== Enums.PresInvisible

			width: Kirigami.Units.gridUnit * 0.2
			height: parent.height

			color: presenceTypeToColor(presenceType)
		}

		// left: avatar
		Item {
			id: avatarSpace
			Layout.preferredHeight: parent.height - Kirigami.Units.gridUnit * 0.8
			Layout.preferredWidth: parent.height - Kirigami.Units.gridUnit * 0.8

			Controls.ToolTip {
				visible: hovered && !Kirigami.Settings.isMobile
				delay: Qt.styleHints.mousePressAndHoldInterval
				text: generateToolTipText(listItem.name, listItem.jid,
				                          listItem.presenceType, listItem.statusMsg)
			}

			RoundImage {
				id: avatar
				visible: avatarImagePath !== ""
				anchors.fill: parent
				source: avatarImagePath
				width: height
				fillMode: Image.PreserveAspectFit
				mipmap: true
			}

			TextAvatar {
				visible: avatarImagePath == ""
				anchors.fill: parent
				width: height
				id: textAvatar
				name: listItem.name
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
						presenceType === Enums.PresError ? // error presence type
						qsTr("Error: Please check the JID.") :
						kaidan.utils.removeNewLinesFromString(lastMessage)
					}
					textFormat: Text.PlainText
					font.pixelSize: Kirigami.Units.gridUnit * 0.8
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
		return type === Enums.PresOnline ? "green" :
			type === Enums.PresChat ? "darkgreen" :
			type === Enums.PresAway ? "orange" :
			type === Enums.PresDND ? "orange" :
			type === Enums.PresXA ? "orange" :
			type === Enums.PresError ? "red" :
			"lightgrey" // unavailable (offline)
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
		string += statusType === Enums.PresOnline ? qsTr("Available") :
		          statusType === Enums.PresChat ? qsTr("Free for chat") :
		          statusType === Enums.PresAway ? qsTr("Away") :
		          statusType === Enums.PresDND ? qsTr("Do not disturb") :
		          statusType === Enums.PresXA ? qsTr("Away for longer") :
		          statusType === Enums.PresUnavailable ? qsTr("Offline") :
		          statusType === Enums.PresError ? qsTr("Error") :
		          "Invalid" // should never be displayed
		string += "</font>"

		// presence status message
		if (statusMsg !== "") {
			string += ": " + statusMsg
		}
		return string
	}
}
