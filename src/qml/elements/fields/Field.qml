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
import org.kde.kirigami 2.12 as Kirigami

/**
 * This is a text field which can be focused and show a hint for invalid input.
 */
ColumnLayout {
	// text of the label for the input field
	property alias labelText: label.text

	// input field
	property alias inputField: inputField

	// placeholder text for the input field
	property alias placeholderText: inputField.placeholderText

	// input method hints for the input field
	property alias inputMethodHints: inputField.inputMethodHints

	// entered text
	property alias text: inputField.text

	// text to be shown as a hint if the entered text is not valid
	property alias invalidHintText: invalidHint.text

	// validity of the entered text
	property bool valid: true

	// requirement for showing the hint for invalid input
	property bool invalidHintMayBeShown: false

	// label for the input field
	Controls.Label {
		id: label
	}

	RowLayout {
		// input field
		Kirigami.ActionTextField {
			id: inputField
			Layout.fillWidth: true
			selectByMouse: true

			// Show a hint for the first time if the entered text is not valid as soon as the input field loses the focus.
			onFocusChanged: {
				if (!focus && !invalidHintMayBeShown) {
					invalidHintMayBeShown = true
					toggleHintForInvalidText()
				}
			}
		}

		// icon for an invalid input
		Kirigami.Icon {
			id: invalidIcon
			visible: invalidHint.visible
			source: "error"
			width: Kirigami.Units.iconSizes.smallMedium
			height: width
		}
	}

	// hint for entering a valid input
	Controls.Label {
		id: invalidHint
		visible: false
		Layout.fillWidth: true
		leftPadding: 5
		rightPadding: 5
		wrapMode: Text.Wrap
		color: Kirigami.Theme.neutralTextColor
	}

	/**
	 * Shows a hint if the entered text is not valid or hides it otherwise.
	 * If invalidHintMayBeShown was initially set to false, that is only done if the input field has lost the focus at least one time because of its onFocusChanged().
	 */
	function toggleHintForInvalidText() {
		invalidHint.visible = !valid && invalidHintMayBeShown && invalidHintText.length > 0;
	}

	/**
	 * Focuses the input field and selects its text.
	 * If the input field is already focused, the focusing is executed again to trigger its onFocusChanged().
	 */
	function forceActiveFocus() {
		if (inputField.focus)
			inputField.focus = false

		inputField.selectAll()
		inputField.forceActiveFocus()
	}
}
