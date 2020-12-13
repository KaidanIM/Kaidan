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
import org.kde.kirigami 2.12 as Kirigami

/**
 * This page is the base of decision pages with two actions.
 *
 * Each action has an own image for describing its purpose.
 */
Kirigami.Page {
	property alias topDescription: topDescription.text
	property alias bottomDescription: bottomDescription.text

	property alias topImageSource: topImage.source
	property alias bottomImageSource: bottomImage.source

	property Kirigami.Action topAction
	property Kirigami.Action bottomAction

	property bool topActionAsMainAction: false

	property int descriptionMargin: 10

	ColumnLayout {
		anchors.fill: parent

		ColumnLayout {
			Layout.maximumWidth: largeButtonWidth
			Layout.alignment:  Qt.AlignCenter

			// image to show above the top action
			Kirigami.Icon {
				id: topImage
				Layout.fillWidth: true
				Layout.fillHeight: true
			}

			// description for the top action
			CenteredAdaptiveText {
				id: topDescription
				Layout.bottomMargin: descriptionMargin
			}

			// button for the top action
			CenteredAdaptiveButton {
				visible: !topActionAsMainAction
				text: topAction.text
				icon.name: topAction.icon.name
				onClicked: topAction.trigger()
				enabled: topAction.enabled
			}

			// button for the top action as main action
			CenteredAdaptiveHighlightedButton {
				visible: topActionAsMainAction
				text: topAction.text
				icon.name: topAction.icon.name
				onClicked: topAction.trigger()
				enabled: topAction.enabled
			}

			// horizontal line to separate the two actions
			Kirigami.Separator {
				Layout.fillWidth: true
				Layout.leftMargin: - (root.width - parent.width) / 4
				Layout.rightMargin: Layout.leftMargin
			}

			// button for the bottom action
			CenteredAdaptiveButton {
				text: bottomAction.text
				icon.name: bottomAction.icon.name
				onClicked: bottomAction.trigger()
				enabled: bottomAction.enabled
			}

			// description for the bottom action
			CenteredAdaptiveText {
				id: bottomDescription
				Layout.topMargin: descriptionMargin
			}

			// image to show below the bottom action
			Kirigami.Icon {
				id: bottomImage
				Layout.fillWidth: true
				Layout.fillHeight: true
			}
		}
	}
}
