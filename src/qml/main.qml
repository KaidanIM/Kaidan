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
import org.kde.kirigami 2.0 as Kirigami
import io.github.kaidanim 1.0

Kirigami.ApplicationWindow {
	id: root
	width: 960
	height: 540

	header: Kirigami.ToolBarApplicationHeader {
		preferredHeight: Kirigami.Units.gridUnit * 2.25
	}

	globalDrawer: Kirigami.GlobalDrawer {
		id: globalDrawer
		title: "Kaidan"
		titleIcon: "kaidan"
		bannerImageSource: kaidan.getResourcePath("images/banner.png");
		// make drawer floating (overlay)
		modal: true
		// start with closed drawer
		drawerOpen: false
		// show open button on the left side
		handleVisible: true

		actions: [
			Kirigami.Action {
				text: qsTr("Logout")
				iconName: "system-shutdown"
				onTriggered: {
					kaidan.mainDisconnect();
					// the login page will be pushed automatically
				}
			},
			Kirigami.Action {
				text: qsTr("About")
				iconName: "help-about"
				onTriggered: {
					// prevent opening the about page multiple times
					while (pageStack.depth > 1) {
						pageStack.pop();
					}
					// open login page
					pageStack.push(aboutPage);
				}
			}
		]
	}

	// when the window was closed, disconnect from jabber server
	onClosing: {
		kaidan.mainDisconnect();
	}

	// load all pages
	Component {id: chatPage; ChatPage {}}
	Component {id: loginPage; LoginPage {}}
	Component {id: rosterPage; RosterPage {}}
	Component {id: aboutPage; AboutPage {}}

	Component.onCompleted: {
		function openLoginPage() {
			// disconnect this func; the login page will do that now
			kaidan.connectionStateDisconnected.disconnect(openLoginPage);

			// close all pages
			while (pageStack.depth > 0) {
				pageStack.pop();
			}

			// open login page   // FIXME: WHY is the login page popped, if only pushed once ?!
			pageStack.push(loginPage, {"isRetry": true});
			pageStack.replace(loginPage, {"isRetry": true});
		}

		if (kaidan.newLoginNeeded()) {
			// open login page and get new data from user
			pageStack.push(loginPage);
		}
		else {
			// open client normally

			// on connection failure, open login page
			kaidan.connectionStateDisconnected.connect(openLoginPage);

			// show roster and login with restored data
			pageStack.push(rosterPage);
			kaidan.mainConnect();
		}
	}
}
