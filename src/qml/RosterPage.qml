/*
 *  Kaidan - A user-friendly XMPP client for every device!
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
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami
import io.github.kaidanim 1.0

Kirigami.ScrollablePage {
	title: qsTr("Contacts")

	RosterAddContactSheet {
		id: addContactSheet
	}
	RosterRemoveContactSheet {
		id: removeContactSheet
		jid: ""
	}

	ListView {
		verticalLayoutDirection: ListView.TopToBottom

		model: kaidan.rosterModel

		delegate: Kirigami.SwipeListItem {
			id: listItem
			height: Kirigami.Units.gridUnit * 2.5

			RowLayout {
				Kirigami.Label {
					Layout.fillWidth: true
					// use the Name or JID
					text: model.name ? model.name : model.jid
				}

				Rectangle {
					id: counterCircle
					visible: model.unreadMessages > 0
					Layout.preferredHeight: Kirigami.Units.gridUnit * 1.25
					Layout.preferredWidth: Kirigami.Units.gridUnit * 1.25
					radius: counterCircle.height * 0.5
					color: "#4CAF50"

					Text {
						id: msgCounter
						text: model.unreadMessages
						color: "white"
						anchors.centerIn: parent
					}
				}

				// placeholder
				Item {
					width: Kirigami.Units.gridUnit * 2
				}
			}

			onClicked: {
				// first push the chat page
				pageStack.push(chatPage, {
					"chatName": (model.name ? model.name : model.jid),
					"recipientJid": model.jid
				});

				// then set the message filter for this jid
				// this will update the unread message count,
				// which will update the roster and will reset the
				// model variable
				kaidan.chatPartner = model.jid;
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
		}
	}

	Component.onCompleted: {
		function openAddContactSheet() {
			addContactSheet.open();
		}
		function disconnectOpenAddContactSheet() {
			addContactDialogRequested.disconnect(openAddContactSheet);
		}
		// open sheet when requested from drawer over signal
		addContactDialogRequested.connect(openAddContactSheet);
		// disconnect the open function, when the roster page is closed
		kaidan.connectionStateDisconnected.connect(disconnectOpenAddContactSheet);
	}
}
