/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

import org.kde.kirigami 2.8 as Kirigami
import im.kaidan.kaidan 1.0
import "settings"

Kirigami.GlobalDrawer {
	id: globalDrawer
	title: Utils.applicationDisplayName()
	titleIcon: Utils.getResourcePath("images/kaidan.svg")
	bannerImageSource: Utils.getResourcePath("images/banner.png")

	SettingsSheet {
		id: settingsSheet
	}

	actions: [
		Kirigami.Action {
			text: qsTr("Invite friends")
			iconName: "mail-invitation"
			onTriggered: {
				Utils.copyToClipboard(Utils.invitationUrl(kaidan.jid))
				passiveNotification(qsTr("Invitation link copied to clipboard"))
			}
		},
		Kirigami.Action {
			text: qsTr("Log out")
			iconName: "system-shutdown"
			onTriggered: {
				closeAdditionalLayers()
				// disconnect (open log in page)
				kaidan.mainDisconnect(true)
			}
		},
		Kirigami.Action {
			text: qsTr("Settings")
			iconName: "settings-configure"
			onTriggered: {
				// open settings page
				if (Kirigami.Settings.isMobile) {
					if (pageStack.layers.depth < 2)
					    pageStack.layers.push(settingsPage)
				} else {
				    settingsSheet.open()
				}
			}
		},
		Kirigami.Action {
			text: qsTr("About")
			iconName: "help-about"
			onTriggered: {
				closeAdditionalLayers()
				// open about sheet
				aboutDialog.open()
			}
		}
	]

	function closeAdditionalLayers() {
		while (pageStack.layers.depth > 1)
			pageStack.layers.pop()
	}
}
