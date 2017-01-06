/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016-2017 LNJ <git@lnj.li>
 *  Copyright (C) 2016 Marzanna
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

import QtQuick 2.0
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ScrollablePage {
	title: "Contacts"

	ListView {
		model: kaidan.rosterController.rosterList

		delegate: Kirigami.SwipeListItem {
			Kirigami.Label {
				// use the Name or JID
				text: name ? name : jid
			}

			onClicked: {
				//kaidan.setCurrentChatPartner(jid)
				pageStack.push(chatPage, {
					"chatName": (name ? name : jid),
					"toJid": jid
				})
			}

			actions: [
				Kirigami.Action {
					iconName: "bookmark-remove"
					onTriggered: {
						print("deleting contact from roster: " + jid)
						//kaidan.removeContactFromRoster()
					}
				}
			]
		}
	}
}
