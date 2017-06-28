/*
 *  Kaidan - Cross platform XMPP client
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
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.0 as Kirigami

Kirigami.OverlaySheet {
	property string jid;

	onSheetOpenChanged: {
		infoLabel.text = qsTr("Do you really want to delete the contact \"%1\" from your roster?").arg(jid);
	}

	ColumnLayout {
		Kirigami.Heading {
			text: qsTr("Delete contact")

			Layout.fillWidth: true
		}

		Kirigami.Label {
			id: infoLabel
			text: ""
			wrapMode: Text.WordWrap

			Layout.fillWidth: true
		}

		RowLayout {
			Layout.topMargin: 10
			Layout.fillWidth: true

			Controls.Button {
				text: qsTr("Cancel")
				onClicked: close()
				Layout.fillWidth: true
			}

			Controls.Button {
				text: qsTr("Delete")
				onClicked: {
					print(jid);
					kaidan.removeContact(jid);
					close();
				}
				Layout.fillWidth: true
			}
		}
	}
}
