/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2017 JBB <jbb.mail@gmx.de>
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

import QtQuick 2.0
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.0 as Kirigami
import "elements"

Kirigami.OverlaySheet {
	id: addContactSheet

	ColumnLayout {
		SheetHeading {
			text: qsTr("Add new contact")
			onCloseRequested: addContactSheet.close()
		}

		Controls.Label {
			text: qsTr("Nickname:")
		}
		Controls.TextField {
			id: nickField
			selectByMouse: true
			Layout.fillWidth: true
		}

		Controls.Label {
			text: qsTr("Jabber-ID:")
		}
		Controls.TextField {
			id: jidField
			placeholderText: qsTr("user@example.org")
			selectByMouse: true
			Layout.fillWidth: true
		}

		RowLayout {
			Layout.topMargin: 10

			Controls.Button {
				text: qsTr("Cancel")
				onClicked: {
					clearInput();
					close();
				}
				Layout.fillWidth: true
			}

			Controls.Button {
				text: qsTr("Add")
				enabled: jidField.length > 0
				onClicked: {
					kaidan.addContact(jidField.text, nickField.text);
					clearInput();
					close();
				}
				Layout.fillWidth: true
			}
		}
	}
	
	function clearInput() {
		jidField.text = "";
		nickField.text = "";
	}
}
