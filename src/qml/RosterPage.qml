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

import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami
import im.kaidan.kaidan 1.0
import "elements"

Kirigami.ScrollablePage {
	title: {
		kaidan.connectionState === Enums.StateConnecting ? qsTr("Connecting…") :
		kaidan.connectionState === Enums.StateDisconnected ? qsTr("Offline") :
		qsTr("Contacts")
	}

	RosterAddContactSheet {
		id: addContactSheet
		jid: ""
	}
	RosterRemoveContactSheet {
		id: removeContactSheet
		jid: ""
	}

	mainAction: Kirigami.Action {
		text: qsTr("Add new contact")
		iconName: "contact-new"
		enabled: !addContactSheet.sheetOpen
		onTriggered: {
			addContactSheet.open();
		}
	}

	ListView {
		verticalLayoutDirection: ListView.TopToBottom
		focus: true
		model: kaidan.rosterModel
		delegate: RosterListItem {
			id: rosterItem
			name: model.name ? model.name : model.jid
			jid: model.jid
			lastMessage: model.lastMessage
			presenceType: kaidan.presenceCache.getDefaultPresType(model.jid)
			statusMsg: kaidan.presenceCache.getDefaultStatus(model.jid)
			unreadMessages: model.unreadMessages
			avatarImagePath: kaidan.avatarStorage.getHashOfJid(model.jid) !== "" ?
					 kaidan.avatarStorage.getAvatarUrl(model.jid) :
					 kaidan.getResourcePath("images/fallback-avatar.svg")
			backgroundColor: {
				kaidan.chatPartner == model.jid ? Kirigami.Theme.highlightColor
				                                : Kirigami.Theme.backgroundColor
			}
			onClicked: {
				// first push the chat page
				pageStack.push(chatPage, {
					"chatName": (model.name ? model.name : model.jid),
					"recipientJid": model.jid
				})

				// then set the message filter for this jid
				// this will update the unread message count,
				// which will update the roster and will reset the
				// model variable
				kaidan.chatPartner = model.jid
			}
			actions: [
				Kirigami.Action {
					iconName: "bookmark-remove"
					onTriggered: {
						removeContactSheet.jid = model.jid;
						removeContactSheet.open();
					}
				}
			]

			function newPresenceArrived(jid) {
				if (jid === model.jid) {
					rosterItem.presenceType = kaidan.presenceCache.
					                          getDefaultPresType(model.jid)
					rosterItem.statusMsg = kaidan.presenceCache.
					                       getDefaultStatus(model.jid)
				}
			}

			function xmppUriReceived(uri) {
				// 'xmpp:' has length of 5
				addContactSheet.jid = uri.substr(5)
				addContactSheet.open()
			}

			Component.onCompleted: {
				kaidan.presenceCache.presenceChanged.connect(newPresenceArrived)
				kaidan.xmppUriReceived.connect(xmppUriReceived)
			}
			Component.onDestruction: {
				kaidan.presenceCache.presenceChanged.disconnect(newPresenceArrived)
				kaidan.xmppUriReceived.disconnect(xmppUriReceived)
			}
		}
	}
}
