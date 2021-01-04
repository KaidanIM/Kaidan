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
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import im.kaidan.kaidan 1.0

import "../elements"
import "../elements/fields"

RowLayout {
	property alias hostField: hostField
	property alias portField: portField
	property CenteredAdaptiveHighlightedButton confirmationButton

	Field {
		id: hostField
		labelText: qsTr("Hostname:")
		placeholderText: "xmpp.example.org"
		text: AccountManager.host

		// Focus the portField on confirmation.
		Keys.onPressed: {
			switch (event.key) {
			case Qt.Key_Return:
			case Qt.Key_Enter:
				portField.forceActiveFocus()
				event.accepted = true
			}
		}
	}

	ColumnLayout {
		Controls.Label {
			text: qsTr("Port:")
		}

		Controls.SpinBox {
			id: portField
			editable: true
			from: AccountManager.nonCustomPort
			to: 65535
			value: AccountManager.port
			Layout.minimumWidth: 80

			textFromValue: function(value, locale) {
				// Return an empty string if no custom port is set.
				if (value === AccountManager.nonCustomPort)
					return ""

				// By returning the value without taking the locale into account, no digit grouping is applied.
				// Example: For a port number of "one thousand" the text "1000" instead of "1,000" is returned.
				return value
			}

			// Allow only an empty string (for no custom port) as input or input from 1 to 99999.
			// Without defining an own validator, zeros at the beginning and dots between the digits and at the end would be valid.
			validator: RegularExpressionValidator {
				regularExpression: /\b(\s|[1-9])[0-9]{4}/
			}

			// Simulate the pressing of the currently clickable confirmation button.
			Keys.onPressed: {
				switch (event.key) {
				case Qt.Key_Return:
				case Qt.Key_Enter:
					// Trigger that the text inside portField is set as its value.
					confirmationButton.forceActiveFocus()

					confirmationButton.clicked()
					event.accepted = true
				}
			}
		}
	}

	function forceFocus() {
		hostField.forceFocus()
	}
}
