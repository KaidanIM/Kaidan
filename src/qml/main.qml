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

import QtQuick 2.6
import QtQuick.Controls.Material 2.0
import org.kde.kirigami 2.0 as Kirigami
import StatusBar 0.1

Kirigami.ApplicationWindow {
	id: root
	width: 960
	height: 540

	// Header / ToolBar
	header: Kirigami.ApplicationHeader {
		preferredHeight: Kirigami.Units.gridUnit * 2.25
	}

	StatusBar {
		color: Material.color(Material.Green, Material.Shade700)
	}

	// Global and Contextual Drawers
	globalDrawer: GlobalDrawer {}
	contextDrawer: Kirigami.ContextDrawer {
		id: contextDrawer
	}

	// when the window was closed, disconnect from jabber server
	onClosing: {
		kaidan.mainDisconnect();
	}

	// load all pages
	Component {id: chatPage; ChatPage {}}
	Component {id: loginPage; LoginPage {}}
	Component {id: rosterPage; RosterPage {}}

	Component.onCompleted: {
		function handleDisconnect() {
			// close all pages
			while (pageStack.depth > 0) {
				pageStack.pop();
			}

			// open login page   // FIXME: WHY is the login page popped, if only pushed once ?!
			pageStack.push(loginPage);
			pageStack.replace(loginPage);
		}

		if (kaidan.newLoginNeeded()) {
			// open login page and get new data from user
			pageStack.push(loginPage);
		} else {
			// open client normally

			// on connection failure, open login page
			kaidan.connectionStateDisconnected.connect(handleDisconnect);

			// show roster and login with restored data
			pageStack.push(rosterPage);
			kaidan.mainConnect();
		}
	}
}
