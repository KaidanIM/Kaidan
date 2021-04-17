/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14 as Controls

import im.kaidan.kaidan 1.0

/**
 * This page is used for the manual registration.
 *
 * Everything can be manually chosen.
 * In case of no input, random values are used.
 * Only required information must be entered to create an account.
 */
RegistrationPage {
	title: qsTr("Register manually")

	// These views are created and inserted into the swipe view dynamically.
	// When they are not required, they are removed.
	// The default ones are added to the SwipeView via Component.onCompleted.
	property WebRegistrationView webRegistrationView
	property LoadingView loadingView
	property UsernameView usernameView
	property PasswordView passwordView
	property CustomFormViewManualRegistration customFormView
	property ResultView resultView

	property bool loadingViewActive: loadingView ? loadingView.Controls.SwipeView.isCurrentItem : false
	property bool jumpingToViewsEnabled: !(loadingViewActive || ((usernameView && usernameView.Controls.SwipeView.isCurrentItem || passwordView && passwordView.Controls.SwipeView.isCurrentItem) && !swipeView.currentItem.valid))
	property bool registrationErrorOccurred: false
	property bool connectionErrorOccurred: false

	property alias displayName: displayNameView.text

	server: serverView.text
	username: usernameView ? usernameView.text : ""
	password: passwordView ? passwordView.text : ""

	ColumnLayout {
		anchors.fill: parent
		spacing: 0

		Controls.SwipeView {
			id: swipeView
			interactive: !loadingViewActive
			Layout.fillWidth: true
			Layout.fillHeight: true
			property int lastIndex: 0

			onCurrentIndexChanged: {
				if (connectionErrorOccurred) {
					connectionErrorOccurred = false
				} else if (!loadingViewActive && currentIndex > lastIndex && currentIndex === serverView.Controls.SwipeView.index + 1 && !webRegistrationView) {
					addLoadingView(currentIndex)
					requestRegistrationForm()
				}

				lastIndex = currentIndex
			}

			DisplayNameView {
				id: displayNameView
			}

			ServerView {
				id: serverView
			}

			// All dynamically loaded views are inserted here when needed.
		}

		NavigationBar {
			id: navigationBar
		}
	}

	Component {id: webRegistrationViewComponent; WebRegistrationView {}}
	Component {id: loadingViewComponent; LoadingView {}}
	Component {id: usernameViewComponent; UsernameView {}}
	Component {id: passwordViewComponent; PasswordView {}}
	Component {id: customFormViewComponent; CustomFormViewManualRegistration {}}
	Component {id: resultViewComponent; ResultView {}}

	Component.onCompleted: addDynamicallyLoadedInBandRegistrationViews()

	Connections {
		target: Kaidan

		function onConnectionErrorChanged() {
			connectionErrorOccurred = true
			jumpToPreviousView()
			removeLoadingView()
		}

		function onRegistrationFormReceived(dataFormModel) {
			formModel = dataFormModel
			formFilterModel.sourceModel = dataFormModel

			var indexToInsert = loadingView.Controls.SwipeView.index

			// There are three cases here:
			//
			// 1. The server did not include a "username" field.
			// The username view needs to be removed.
			if (!formModel.hasUsernameField()) {
				swipeView.removeItem(usernameView)
			// 2. The server did include a "username" field, but the server selected before did not include it and the username view has been removed.
			// The view needs to be added again.
			} else if (!usernameView) {
				addUsernameView(++indexToInsert)
			// 3. The server did include a "username" field and the username view is already loaded.
			} else {
				indexToInsert++
			}

			// Same logic as for the username view. See above.
			if (!formModel.hasPasswordField()) {
				swipeView.removeItem(passwordView)
			} else if (!passwordView) {
				addPasswordView(++indexToInsert)
			} else {
				indexToInsert++
			}

			// Same logic as for the username view. See above.
			if (!customFormFieldsAvailable()) {
				swipeView.removeItem(customFormView)
			} else if (!customFormView) {
				addCustomFormView(++indexToInsert)
			} else {
				indexToInsert++
			}

			// Only jump to the next view if the registration form was not loaded because a registration error occurred.
			// Depending on the error, the swipe view jumps to a particular view (see onRegistrationFailed).
			if (registrationErrorOccurred)
				registrationErrorOccurred = false
			else if (serverView.Controls.SwipeView.isPreviousItem)
				jumpToNextView()

			removeLoadingView()
			focusFieldViews()
		}

		function onRegistrationOutOfBandUrlReceived(outOfBandUrl) {
			serverView.outOfBandUrl = outOfBandUrl
			handleInBandRegistrationNotSupported()
		}

		// Depending on the error, the swipe view jumps to the view where the input should be corrected.
		// For all remaining errors, the swipe view jumps to the server view.
		function onRegistrationFailed(error, errorMessage) {
			registrationErrorOccurred = true

			switch(error) {
			case RegistrationManager.InBandRegistrationNotSupported:
				handleInBandRegistrationNotSupported()
				break
			case RegistrationManager.UsernameConflict:
				requestRegistrationForm()
				handleUsernameConflictError()
				jumpToView(usernameView)
				break
			case RegistrationManager.PasswordTooWeak:
				requestRegistrationForm()
				passiveNotification(qsTr("The server requires a stronger password."))
				jumpToView(passwordView)
				break
			case RegistrationManager.CaptchaVerificationFailed:
				requestRegistrationForm()
				showPassiveNotificationForCaptchaVerificationFailedError()
				jumpToView(customFormView)
				break
			case RegistrationManager.RequiredInformationMissing:
				requestRegistrationForm()
				if (customFormView) {
					showPassiveNotificationForRequiredInformationMissingError(errorMessage)
					jumpToView(customFormView)
				} else {
					showPassiveNotificationForUnknownError(errorMessage)
				}
				break
			default:
				requestRegistrationForm()
				showPassiveNotificationForUnknownError(errorMessage)
				jumpToView(serverView)
			}
		}
	}

	// Simulate the pressing of the currently clickable confirmation button.
	Keys.onPressed: {
		switch (event.key) {
		case Qt.Key_Return:
		case Qt.Key_Enter:
			if (resultView && resultView.Controls.SwipeView.isCurrentItem)
				resultView.registrationButton.clicked()
			else if (jumpingToViewsEnabled)
				navigationBar.nextButton.clicked()
		}
	}

	/**
	 * Shows a passive notification regarding the missing support of In-Band Registration.
	 * If the server supports web registration, the corresponding view is opened.
	 * If the server does not support web registration and it is not a custom server, another one is automatically selected.
	 */
	function handleInBandRegistrationNotSupported() {
		var notificationText = serverView.customServerSelected ? qsTr("The server does not support registration via this app.") : qsTr("The server does currently not support registration via this app.")

		if (serverView.registrationWebPage || serverView.outOfBandUrl) {
			addWebRegistrationView()
			notificationText += " " + qsTr("But you can use the server's web registration.")
		} else {
			if (!serverView.customServerSelected) {
				serverView.selectServerRandomly()
				notificationText += " " + qsTr("A new server has been randomly selected.")
			}

			jumpToPreviousView()
		}

		passiveNotification(notificationText)
		removeLoadingView()
	}

	/**
	 * Shows a passive notification if a username is already taken on the server.
	 * If a random username was used for registration, a new one is generated.
	 */
	function handleUsernameConflictError() {
		var notificationText = qsTr("The username is already taken.")

		if (usernameView.enteredText.length === 0) {
			usernameView.regenerateUsername()
			notificationText += " " + qsTr("A new random username has been generated.")
		}

		passiveNotification(notificationText)
	}

	/**
	 * Focuses the input field of the currently shown field view.
	 *
	 * This is necessary to execute after a registration form is received because the normal focussing within FieldView does not work then.
	 */
	function focusFieldViews() {
		if (swipeView.currentItem === usernameView || swipeView.currentIndex === passwordView || swipeView.currentIndex === customFormView)
			swipeView.currentItem.forceActiveFocus()
	}

	/**
	 * Adds the web registration view to the swipe view.
	 */
	function addWebRegistrationView() {
		removeDynamicallyLoadedInBandRegistrationViews()

		webRegistrationView = webRegistrationViewComponent.createObject(swipeView)
		swipeView.insertItem(serverView.Controls.SwipeView.index + 1, webRegistrationView)
	}

	/**
	 * Removes the web registration view from the swipe view.
	 */
	function removeWebRegistrationView() {
		if (webRegistrationView) {
			swipeView.removeItem(webRegistrationView)
			addDynamicallyLoadedInBandRegistrationViews()
		}
	}

	/**
	 * Adds the dynamically loaded views used for the In-Band Registration to the swipe view.
	 */
	function addDynamicallyLoadedInBandRegistrationViews() {
		var indexToInsert = serverView.Controls.SwipeView.index

		addUsernameView(++indexToInsert)
		addPasswordView(++indexToInsert)
		addCustomFormView(++indexToInsert)
		addResultView(++indexToInsert)
	}

	/**
	 * Removes the dynamically loaded views from the swipe view.
	 */
	function removeDynamicallyLoadedInBandRegistrationViews() {
		for (var view of [usernameView, passwordView, customFormView, resultView]) {
			swipeView.removeItem(view)
		}
	}

	/**
	 * Adds the loading view to the swipe view.
	 *
	 * @param index index of the swipe view at which the loading view will be inserted
	 */
	function addLoadingView(index) {
		loadingView = loadingViewComponent.createObject(swipeView)
		swipeView.insertItem(index, loadingView)
	}

	/**
	 * Removes the loading view from the swipe view after jumping to the next page.
	 */
	function removeLoadingView() {
		swipeView.removeItem(loadingView)
	}

	/**
	 * Adds the username view to the swipe view.
	 *
	 * @param index position in the swipe view to insert the username view
	 */
	function addUsernameView(index) {
		usernameView = usernameViewComponent.createObject(swipeView)
		swipeView.insertItem(index, usernameView)
	}

	/**
	 * Adds the password view to the swipe view.
	 *
	 * @param index position in the swipe view to insert the password view
	 */
	function addPasswordView(index) {
		passwordView = passwordViewComponent.createObject(swipeView)
		swipeView.insertItem(index, passwordView)
	}

	/**
	 * Adds the custom form view to the swipe view.
	 *
	 * @param index position in the swipe view to insert the custom form view
	 */
	function addCustomFormView(index) {
		customFormView = customFormViewComponent.createObject(swipeView)
		swipeView.insertItem(index, customFormView)
	}

	/**
	 * Adds the result view to the swipe view.
	 *
	 * @param index position in the swipe view to insert the result view
	 */
	function addResultView(index) {
		resultView = resultViewComponent.createObject(swipeView)
		swipeView.insertItem(index, resultView)
	}

	/**
	 * Jumps to the previous view.
	 */
	function jumpToPreviousView() {
		swipeView.decrementCurrentIndex()
	}

	/**
	 * Jumps to the next view.
	 */
	function jumpToNextView() {
		swipeView.incrementCurrentIndex()
	}

	/**
	 * Jumps to a given view.
	 *
	 * @param view view to be jumped to
	 */
	function jumpToView(view) {
		swipeView.setCurrentIndex(view.Controls.SwipeView.index)
	}

	/**
	 * Requests a registration and shows the loading view.
	 */
	function sendRegistrationFormAndShowLoadingView() {
		addLoadingView(swipeView.currentIndex + 1)
		jumpToNextView()

		Kaidan.client.vCardManager.changeDisplayNameRequested(displayName)
		sendRegistrationForm()
	}
}
