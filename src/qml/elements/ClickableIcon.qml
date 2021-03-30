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
import QtQuick.Controls 2.14 as Controls
import org.kde.kirigami 2.12 as Kirigami

/**
 * This is a clickable icon changing its color on several events if possible.
 */
Kirigami.Icon  {
	width: Kirigami.Units.largeSpacing * 3.8
	height: width

	// Icons which cannot be colored (e.g. emojis) are highlighted in another way.
	active: mouseArea.containsMouse

	signal clicked()

	MouseArea {
		id: mouseArea
		anchors.fill: parent
		hoverEnabled: true
		onEntered: parent.color = Kirigami.Theme.hoverColor
		onPressed: parent.color = Kirigami.Theme.focusColor
		onExited: parent.color = Kirigami.Theme.textColor

		onClicked: {
			parent.clicked()

			if (containsMouse)
				entered()
			else
				exited()
		}
	}
}
