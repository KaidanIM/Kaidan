/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami

Kirigami.SwipeListItem {
	property string name;
	property string lastMessage;
	property int unreadMessages;
	property string avatarImagePath;

	id: listItem
	topPadding: Kirigami.Units.smallSpacing * 1.5
	height: Kirigami.Units.gridUnit * 3.5

	RowLayout {
		spacing: Kirigami.Units.gridUnit * 0.5

		// left side: Avatar
		RoundImage {
			source: avatarImagePath
			width: height
			fillMode: Image.PreserveAspectFit
			Layout.preferredHeight: parent.height
			Layout.preferredWidth: parent.height
			mipmap: true
		}
		// right side
		ColumnLayout {
			// top
			RowLayout {
				// contact name
				Kirigami.Heading {
					text: name
					level: 3
					Layout.fillWidth: true
				}
			}
			// bottom
			Kirigami.Label {
				Layout.fillWidth: true
				text: kaidan.removeNewLinesFromString(lastMessage);
			}
		}

		// unread message counter
		MessageCounter {
			visible: unreadMessages > 0
			counter: unreadMessages

			Layout.preferredHeight: Kirigami.Units.gridUnit * 1.25
			Layout.preferredWidth: Kirigami.Units.gridUnit * 1.25
		}

		// placeholder
		Item {
			width: Kirigami.Units.gridUnit * 2.5
		}
	}
}
