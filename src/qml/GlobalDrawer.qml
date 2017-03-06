/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 JBBgameich <jbb.mail@gmx.de>
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

import org.kde.kirigami 2.0 as Kirigami

Kirigami.GlobalDrawer {
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
