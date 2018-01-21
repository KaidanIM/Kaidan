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
			spacing: Kirigami.Units.smallSpacing

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
			Controls.Label {
				Layout.fillWidth: true
				elide: Text.ElideRight
				maximumLineCount: 1
				text: kaidan.removeNewLinesFromString(lastMessage);
				textFormat: Text.PlainText
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
