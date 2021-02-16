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

import QtQuick 2.14
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import QtGraphicalEffects 1.12
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0

Kirigami.SwipeListItem {
	id: root

	property string jid
	property string name
	property string lastMessage
	property int unreadMessages
	property bool isSelected: !Kirigami.Settings.isMobile && MessageModel.currentChatJid === jid

	topPadding: 0
	leftPadding: 0
	bottomPadding: 0
	height: 65
	backgroundColor: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor

	RowLayout {
		spacing: Kirigami.Units.gridUnit * 0.5

		// left border: presence
		Rectangle {
			id: presenceIndicator

			width: Kirigami.Units.gridUnit * 0.3
			height: parent.height

			color: userPresence.availabilityColor

			UserPresenceWatcher {
				id: userPresence
				jid: root.jid
			}
		}

		// left: avatar
		Item {
			Layout.preferredHeight: parent.height - Kirigami.Units.gridUnit * 0.8
			Layout.preferredWidth: parent.height - Kirigami.Units.gridUnit * 0.8

			Avatar {
				anchors.fill: parent
				avatarUrl: Kaidan.avatarStorage.getAvatarUrl(jid)
				name: root.name
				width: height
				isBorderVisible: root.hovered || root.isSelected
			}
		}

		// right side
		ColumnLayout {
			spacing: Kirigami.Units.smallSpacing
			Layout.fillWidth: true

			// contact name
			RowLayout {
				Kirigami.Heading {
					text: name
					textFormat: Text.PlainText
					elide: Text.ElideRight
					maximumLineCount: 1
					level: 3
					Layout.fillWidth: true
					Layout.maximumHeight: Kirigami.Units.gridUnit * 1.5
				}
				// muted-icon
				Kirigami.Icon {
					id: muteIcon
					source: "audio-volume-muted-symbolic"
					width: 22
					height: 22
					visible: Kaidan.notificationsMuted(jid)
				}
				Item {
					Layout.fillWidth: true
				}
			}
			// bottom
			RowLayout {
				Layout.fillWidth: true

				Controls.Label {
					Layout.fillWidth: true
					elide: Text.ElideRight
					maximumLineCount: 1
					text: Utils.removeNewLinesFromString(lastMessage)
					textFormat: Text.PlainText
					font.pointSize: Kirigami.Units.gridUnit * 0.58
				}
			}
		}

		// unread message counter
		MessageCounter {
			id: counter
			visible: unreadMessages > 0
			counter: unreadMessages
			muted: Kaidan.notificationsMuted(jid)

			Layout.preferredHeight: Kirigami.Units.gridUnit * 1.25
			Layout.preferredWidth: Kirigami.Units.gridUnit * 1.25
		}

		Connections {
			target: Kaidan

			function onNotificationsMutedChanged(jid) {
				if (jid === root.jid) {
					counter.muted = Kaidan.notificationsMuted(jid)
					muteIcon.visible = Kaidan.notificationsMuted(jid)
				}
			}
		}
	}
}
