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
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0

/**
 * This is the base of a registration page.
 */
Kirigami.Page {
	// This model contains all fields from the registration form of the requested server.
	property DataFormModel formModel

	// This model only contains the custom fields from the registration form of the requested server.
	// It may contain e.g. a CAPTCHA or an email address.
	// The server may not use the standard way for requesting the username and the password.
	// In that case, this model could also include fields for those values.
	property alias formFilterModel: formFilterModel

	// generator for random usernames and passwords
	property alias credentialsGenerator: credentialsGenerator

	// JID of the server from whom the registration form is requested
	property string server

	// username of the user to be registered
	property string username

	// password of the user to be registered
	property string password

	leftPadding: 0
	topPadding: 0
	rightPadding: 0
	bottomPadding: 0

	RegistrationDataFormFilterModel {
		id: formFilterModel
	}

	CredentialsGenerator {
		id: credentialsGenerator
	}

	Component.onCompleted: AccountManager.resetCustomConnectionSettings()

	/**
	 * Shows a passive notification if the CAPTCHA verification failed.
	 */
	function showPassiveNotificationForCaptchaVerificationFailedError() {
		passiveNotification(qsTr("CAPTCHA input was wrong"))
	}

	/**
	 * Shows a passive notification if required information is missing.
	 *
	 * @param errorMessage text describing the error and the required missing information
	 */
	function showPassiveNotificationForRequiredInformationMissingError(errorMessage) {
		passiveNotification(qsTr("Required information is missing: ") + errorMessage)
	}

	/**
	 * Shows a passive notification for an unknown error.
	 *
	 * @param errorMessage text describing the error
	 */
	function showPassiveNotificationForUnknownError(errorMessage) {
		passiveNotification(qsTr("Registration failed:") + " " + errorMessage)
	}

	/**
	 * Returns true if the registration form received from the server contains custom fields.
	 */
	function customFormFieldsAvailable() {
		return formFilterModel.rowCount() > 0
	}

	/**
	 * Requests a registration form from the server.
	 */
	function requestRegistrationForm() {
		// Set the server's JID.
		AccountManager.jid = server

		// Request a registration form.
		Kaidan.requestRegistrationForm()
	}

	/**
	 * Sends the completed registration form to the server.
	 */
	function sendRegistrationForm() {
		if (formModel.hasUsernameField())
			formModel.setUsername(username)
		if (formModel.hasPasswordField())
			formModel.setPassword(password)

		Kaidan.sendRegistrationForm()
	}
}
