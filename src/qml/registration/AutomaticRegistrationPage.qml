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
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0
import "../elements"

/**
 * This page is used for the automatic registration.
 *
 * Everything that can be automated is automated.
 * The server is randomly selected.
 * The username and password are randomly generated.
 * Therefore, only custom information requested by the server is shown.
 * Only required information must be entered to register an account.
 */
RegistrationPage {
	title: qsTr("Register automatically")

	property int maximumAttemptsForRequestingRegistrationFormFromAnotherServer: 3
	property int remainingAttemptsForRequestingRegistrationFormFromAnotherServer: maximumAttemptsForRequestingRegistrationFormFromAnotherServer

	property int maximumRegistrationAttemptsAfterUsernameConflictOccurred: 2
	property int remainingRegistrationAttemptsAfterUsernameConflictOccurred: maximumRegistrationAttemptsAfterUsernameConflictOccurred

	property int maximumRegistrationAttemptsAfterCaptchaVerificationFailedOccurred: 2
	property int remainingRegistrationAttemptsAfterCaptchaVerificationFailedOccurred: maximumRegistrationAttemptsAfterCaptchaVerificationFailedOccurred

	property int maximumRegistrationAttemptsAfterRequiredInformationMissingOccurred: 2
	property int remainingRegistrationAttemptsAfterRequiredInformationMissingOccurred: maximumRegistrationAttemptsAfterRequiredInformationMissingOccurred

	ServerListModel {
		id: serverListModel
	}

	ColumnLayout {
		anchors.fill: parent

		Controls.StackView {
			id: stackView
			clip: true
			Layout.fillWidth: true
			Layout.fillHeight: true
			initialItem: loadingViewComponent
		}
	}

	Component { id: loadingViewComponent; LoadingView {} }

	Component { id: customFormViewComponent; CustomFormViewAutomaticRegistration {}	}

	Component.onCompleted: {
		chooseServerRandomly()
		chooseUsernameRandomly()
		choosePasswordRandomly()
		requestRegistrationForm()
	}

	Connections {
		target: Kaidan

		onRegistrationFormReceived: {
			formModel = dataFormModel
			formFilterModel.sourceModel = dataFormModel

			// If the received registration data form does not contain custom fields, request the registration directly.
			// Otherwise, remove the loading view to show the custom form view.
			if (!customFormFieldsAvailable())
				sendRegistrationForm()
			else
				removeLoadingView()
		}

		onConnectionErrorChanged: {
			showPassiveNotificationForConnectionError()
			removeLoadingView()
			popLayerIfNoCustomFormFieldsAvailable()
		}

		onRegistrationFailed: {
			switch (error) {
			case RegistrationManager.InBandRegistrationNotSupported:
				requestRegistrationFormFromAnotherServer(errrorMessage)
				break
			case RegistrationManager.UsernameConflict:
				if (remainingRegistrationAttemptsAfterUsernameConflictOccurred > 0) {
					remainingRegistrationAttemptsAfterUsernameConflictOccurred--
					// Try to register again with another username on the same server.
					chooseUsernameRandomly()
					requestRegistrationForm()
				} else {
					remainingRegistrationAttemptsAfterUsernameConflictOccurred = maximumRegistrationAttemptsAfterUsernameConflictOccurred
					requestRegistrationFormFromAnotherServer(errrorMessage)
				}
				break
			case RegistrationManager.CaptchaVerificationFailed:
				if (remainingRegistrationAttemptsAfterCaptchaVerificationFailedOccurred > 0) {
					remainingRegistrationAttemptsAfterCaptchaVerificationFailedOccurred--
					requestRegistrationForm()
					showPassiveNotificationForCaptchaVerificationFailedError()
				} else {
					remainingRegistrationAttemptsAfterCaptchaVerificationFailedOccurred = maximumRegistrationAttemptsAfterCaptchaVerificationFailedOccurred
					requestRegistrationFormFromAnotherServer(errrorMessage)
				}
				break
			case RegistrationManager.RequiredInformationMissing:
				if (remainingRegistrationAttemptsAfterRequiredInformationMissingOccurred > 0 && customFormFieldsAvailable()) {
					remainingRegistrationAttemptsAfterRequiredInformationMissingOccurred--
					requestRegistrationForm()
					showPassiveNotificationForRequiredInformationMissingError(errrorMessage)
				} else {
					remainingRegistrationAttemptsAfterRequiredInformationMissingOccurred = remainingRegistrationAttemptsAfterRequiredInformationMissingOccurred
					requestRegistrationFormFromAnotherServer(errrorMessage)
				}
				break
			default:
				requestRegistrationFormFromAnotherServer(errrorMessage)
			}
		}
	}

	// Simulate the pressing of the confirmation button.
	Keys.onPressed: {
		if (stackView.currentItem.registrationButton && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter))
			stackView.currentItem.registrationButton.clicked()
	}

	/**
	 * Adds the loading view to the stack view.
	 */
	function addLoadingView() {
		stackView.push(loadingViewComponent)
	}

	/**
	 * Removes the loading view from the stack view.
	 */
	function removeLoadingView() {
		// Push the custom form view on the stack view if the loading view is the only item on it.
		// That is the case during the first registration form request.
		// When the loading view was pushed on the stack view after submitting custom form fields and the registration fails or a connection error occurs, that loading view is popped to show the custom form view again.
		if (stackView.depth === 1) {
			stackView.push(customFormViewComponent)
			stackView.currentItem.registrationButton.parent = stackView.currentItem.contentArea
		} else {
			stackView.pop()
		}
	}

	/**
	 * Sets a randomly chosen server for registration.
	 */
	function chooseServerRandomly() {
		server = serverListModel.data(serverListModel.randomlyChooseIndex(), ServerListModel.JidRole)
	}

	/**
	 * Sets a randomly generated username for registration.
	 */
	function chooseUsernameRandomly() {
		username = credentialsGenerator.generateUsername()
	}

	/**
	 * Sets a randomly generated password for registration.
	 */
	function choosePasswordRandomly() {
		password = credentialsGenerator.generatePassword()
	}

	/**
	 * Requests a registration form from another randomly chosen server if an error occurred.
	 *
	 * That is done multiple times and after a maximum number of attempts, the user has to intervene.
	 *
	 * @param errrorMessage message describing the error
	 */
	function requestRegistrationFormFromAnotherServer(errrorMessage) {
		if (remainingAttemptsForRequestingRegistrationFormFromAnotherServer > 0) {
			remainingAttemptsForRequestingRegistrationFormFromAnotherServer--
			chooseServerRandomly()
			requestRegistrationForm()
		} else {
			remainingAttemptsForRequestingRegistrationFormFromAnotherServer = maximumAttemptsForRequestingRegistrationFormFromAnotherServer
			showPassiveNotificationForUnknownError(errrorMessage)
			popLayerIfNoCustomFormFieldsAvailable()
		}
	}

	/**
	 * Sends the completed registration form and adds the loading view.
	 */
	function sendRegistrationFormAndShowLoadingView() {
		sendRegistrationForm()
		addLoadingView()
	}

	/**
	 * Pops this layer if the received registration form does not contain any custom field.
	 */
	function popLayerIfNoCustomFormFieldsAvailable() {
		if (!customFormFieldsAvailable())
			pageStack.layers.pop()
	}
}
