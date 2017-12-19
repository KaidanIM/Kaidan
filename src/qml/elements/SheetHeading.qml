/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2017 LNJ <git@lnj.li>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0 as Controls
import org.kde.kirigami 2.0 as Kirigami

RowLayout {
	id: sheetHeading
	property string text;
	signal closeRequested();

	Kirigami.Heading {
		text: sheetHeading.text
		Layout.fillWidth: true
	}
	Controls.ToolButton {
		text: "\u2715" // Unicode MULTIPLICATION X
		onClicked: {
			sheetHeading.closeRequested();
		}
		Layout.alignment: Qt.AlignRight | Qt.AlignTop
	}
}
