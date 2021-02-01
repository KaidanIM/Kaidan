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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0

/**
 * This page is the base for pages with content needing an explanation.
 *
 * It consists of a centered content area and an overlay containing an explanation area with a background and two buttons.
 */
Kirigami.Page {
	leftPadding: 0
	rightPadding: 0
	topPadding: 0
	bottomPadding: 0

	/**
	 * area containing the explanation displayed while the content is not displayed
	 */
	property alias explanationArea: explanationArea

	/**
	 * explanation within the explanation area
	 */
	property alias explanation: explanationArea.data

	/**
	 * content displayed while the explanation is not displayed
	 */
	property alias content: contentArea.data

	/**
	 * button for a primary action
	 */
	property alias primaryButton: primaryButton

	/**
	 * button for a secondary action
	 */
	property alias secondaryButton: secondaryButton

	/**
	 * true to have a margin between the content and the window's border, otherwise false
	 */
	property bool useMarginsForContent: true

	Item {
		anchors.fill: parent

		GridLayout {
			id: contentArea
			anchors.fill: parent
			anchors.margins: useMarginsForContent ? 20 : 0
			anchors.bottomMargin: useMarginsForContent ? parent.height - buttonArea.y : 0
		}

		// background of overlay
		Rectangle {
			z: 1
			anchors.fill: overlay
			anchors.margins: -8
			radius: roundedCornersRadius
			color: Kirigami.Theme.backgroundColor
			opacity: 0.90
			visible: explanationArea.visible
		}

		ColumnLayout {
			id: overlay
			z: 2
			anchors.margins: 18
			anchors.top: parent.top
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom

			GridLayout {
				id: explanationArea
				Layout.fillWidth: true
				Layout.fillHeight: true
			}

			ColumnLayout {
				id: buttonArea
				Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
				Layout.maximumWidth: largeButtonWidth

				CenteredAdaptiveHighlightedButton {
					id: primaryButton
				}

				CenteredAdaptiveButton {
					id: secondaryButton
				}
			}
		}
	}
}
