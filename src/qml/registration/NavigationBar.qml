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

/**
 * This is the navigation bar for the swipe view.
 *
 * It contains buttons for jumping to the previous and the next view.
 * In between the navigation buttons there is an indicator for the current view.
 */
RowLayout {
	Layout.fillWidth: true
	Layout.margins: 15

	property alias nextButton: nextButton

	// button for jumping to the previous view
	Controls.RoundButton {
		id: previousButton
		Layout.alignment: Qt.AlignLeft
		icon.name: "go-previous-symbolic"
		highlighted: true
		visible: swipeView.currentIndex !== 0
		enabled: jumpingToViewsEnabled
		onClicked: jumpToPreviousView()
	}

	// placeholder for the previous button when it is invisible
	Item {
		width: previousButton.width
		height: previousButton.height
		visible: !previousButton.visible
	}

	// placeholder
	Item {
		Layout.fillWidth: true
		width: {
			if (previousButton.visible)
				return previousButton.width
		}
	}

	// indicator for showing the current postion (index) of the siwpe view
	Controls.PageIndicator {
		id: indicator
		Layout.alignment: Qt.AlignCenter

		count: swipeView.count
		currentIndex: swipeView.currentIndex
	}

	// placeholder
	Item {
		Layout.fillWidth: true
	}

	// placeholder for the next button when it is invisible
	Item {
		width: nextButton.width
		height: nextButton.height
		visible: !nextButton.visible
	}

	// button for jumping to the next view
	Controls.RoundButton {
		id: nextButton
		Layout.alignment: Qt.AlignRight
		icon.name: "go-next-symbolic"
		highlighted: true
		visible: swipeView.currentIndex !== (swipeView.count - 1)
		enabled: jumpingToViewsEnabled
		onClicked: jumpToNextView()
	}
}
