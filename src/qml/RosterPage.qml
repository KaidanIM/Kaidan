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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as Controls
import org.kde.kirigami 2.8 as Kirigami
import im.kaidan.kaidan 1.0
import "elements"

Kirigami.ScrollablePage {
	title: {
		Kaidan.connectionState === Enums.StateConnecting ? qsTr("Connecting…") :
		Kaidan.connectionState === Enums.StateDisconnected ? qsTr("Offline") :
		qsTr("Contacts")
	}
	leftPadding: 0
	topPadding: 0
	rightPadding: 0
	bottomPadding: 0

	RosterAddContactSheet {
		id: addContactSheet
		jid: ""
	}

	mainAction: Kirigami.Action {
		text: qsTr("Add new contact")
		icon.name: "contact-new"
		onTriggered: {
			if (addContactSheet.sheetOpen)
				addContactSheet.close()
			else
				addContactSheet.open()
		}
	}

	rightAction: Kirigami.Action {
		id: searchAction
		text: qsTr("Search contacts")
		checkable: true
		icon.name: "search"
		onTriggered: {
			if (checked) {
				searchField.forceActiveFocus()
				searchField.selectAll()
			}
		}
		shortcut: "Ctrl+F"
	}

	header: Item {
		height: searchField.visible ? searchField.height : 0
		Kirigami.SearchField {
			id: searchField
			focusSequence: ""
			width: parent.width
			height: Kirigami.Units.gridUnit * 2
			visible: searchAction.checked
			onVisibleChanged: text = ""
			onTextChanged: filterModel.setFilterFixedString(text.toLowerCase())
		}
	}

	ListView {
		verticalLayoutDirection: ListView.TopToBottom
		model: RosterFilterProxyModel {
			id: filterModel
			sourceModel: Kaidan.rosterModel
		}
		delegate: RosterListItem {
			id: rosterItem
			height: 65
			name: model.name ? model.name : model.jid
			jid: model.jid
			lastMessage: model.lastMessage
			presenceType: Kaidan.presenceCache.getPresenceType(model.jid)
			statusMsg: Kaidan.presenceCache.getStatusText(model.jid)
			unreadMessages: model.unreadMessages
			avatarImagePath: Kaidan.avatarStorage.getAvatarUrl(model.jid)
			backgroundColor: {
				if (!Kirigami.Settings.isMobile &&
						Kaidan.messageModel.chatPartner === model.jid) {
					Kirigami.Theme.highlightColor
				} else {
					Kirigami.Theme.backgroundColor
				}
			}
			onClicked: {
				searchAction.checked = false
				// We need to cache the chatName, because changing the chatPartner in the
				// message model will in some cases also update the roster model. That
				// will then remove this item and readd an updated version of it, so
				// model.* won't work anymore after this.
				var chatName = model.name ? model.name : model.jid
				Kaidan.messageModel.chatPartner = model.jid
				pageStack.push(chatPage, {
					"chatName": chatName
				})
			}

			function newPresenceArrived(jid) {
				if (jid === model.jid) {
					rosterItem.presenceType = Kaidan.presenceCache.getPresenceType(model.jid)
					rosterItem.statusMsg = Kaidan.presenceCache.getStatusText(model.jid)
				}
			}

			function xmppUriReceived(uri) {
				// 'xmpp:' has length of 5
				addContactSheet.jid = uri.substr(5)
				addContactSheet.open()
			}

			Component.onCompleted: {
				Kaidan.presenceCache.presenceChanged.connect(newPresenceArrived)
				Kaidan.xmppUriReceived.connect(xmppUriReceived)
			}
			Component.onDestruction: {
				Kaidan.presenceCache.presenceChanged.disconnect(newPresenceArrived)
				Kaidan.xmppUriReceived.disconnect(xmppUriReceived)
			}
		}
	}
}
