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

import QtQuick 2.7
import QtQuick.Controls.Material 2.3
import org.kde.kirigami 2.8 as Kirigami
import StatusBar 0.1

import im.kaidan.kaidan 1.0

import "elements"
import "settings"

Kirigami.ApplicationWindow {
	id: root

	minimumHeight: 300
	minimumWidth: 280

	// radius for using rounded corners
	readonly property int roundedCornersRadius: Kirigami.Units.smallSpacing * 1.5

	StatusBar {
		color: Material.color(Material.Green, Material.Shade700)
	}

	// Global and Contextual Drawers
	globalDrawer: GlobalDrawer {}
	contextDrawer: Kirigami.ContextDrawer {
		id: contextDrawer
	}

	AboutDialog {
		id: aboutDialog
		focus: true
		x: (parent.width - width) / 2
		y: (parent.height - height) / 2
	}

	SubRequestAcceptSheet {
		id: subReqAcceptSheet
	}

	// when the window was closed, disconnect from jabber server
	onClosing: {
		kaidan.mainDisconnect()
	}

	// load all pages
	Component {id: chatPage; ChatPage {}}
	Component {id: loginPage; LoginPage {}}
	Component {id: rosterPage; RosterPage {}}
	Component {id: emptyChatPage; EmptyChatPage {}}
	Component {id: settingsPage; SettingsPage {}}
	Component {id: qrCodeScannerPage; QrCodeScannerPage {}}
	Component {id: userProfilePage; UserProfilePage {}}
	Component {id: multimediaSettingsPage; MultimediaSettingsPage {}}

	/**
	 * Shows a passive notification for a long period.
	 */
	function passiveNotification(text) {
		showPassiveNotification(text, "long")
	}

	function showPassiveNotificationForConnectionError() {
		passiveNotification(Utils.connectionErrorMessage(kaidan.connectionError))
	}

	function openLoginPage() {
		globalDrawer.enabled = false
		globalDrawer.visible = false

		popLayersAboveLowest()
		popAllPages()
		pageStack.push(loginPage)
	}

	/**
	 * Opens the view with the roster and chat page.
	 */
	function openChatView() {
		globalDrawer.enabled = true

		popAllPages()
		pageStack.push(rosterPage)
		if (!Kirigami.Settings.isMobile)
			pageStack.push(emptyChatPage)
	}

	/**
	 * Pops all layers except the layer with index 0 from the page stack.
	 */
	function popLayersAboveLowest() {
		while (pageStack.layers.depth > 1)
			pageStack.layers.pop()
	}

	/**
	 * Pops all pages from the page stack.
	 */
	function popAllPages() {
		while (pageStack.depth > 0)
			pageStack.pop()
	}

	function handleSubRequest(from, message) {
		kaidan.vCardRequested(from)

		subReqAcceptSheet.from = from
		subReqAcceptSheet.message = message

		subReqAcceptSheet.open()
	}

	Component.onCompleted: {
		kaidan.passiveNotificationRequested.connect(passiveNotification)
		kaidan.newCredentialsNeeded.connect(openLoginPage)
		kaidan.logInWorked.connect(openChatView)
		kaidan.subscriptionRequestReceived.connect(handleSubRequest)

		openChatView()

		// Announce that the user interface is ready and the application can start connecting.
		kaidan.start()
	}

	Component.onDestruction: {
		kaidan.passiveNotificationRequested.disconnect(passiveNotification)
		kaidan.newCredentialsNeeded.disconnect(openLoginPage)
		kaidan.logInWorked.disconnect(openChatView)
		kaidan.subscriptionRequestReceived.disconnect(handleSubRequest)
	}
}
