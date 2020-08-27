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

import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.2 as Controls
import org.kde.kirigami 2.10 as Kirigami

import im.kaidan.kaidan 1.0

Kirigami.ScrollablePage {
	title: qsTr("Connection settings")

	ColumnLayout {
		spacing: 20
		Kirigami.InlineMessage {
			Layout.fillWidth: true
			visible: true
			text: qsTr("The connection settings will be saved permanently after the first successful login."
					   + " If they don't work, you'll get back to the login page.")
		}

		Kirigami.FormLayout {
			Layout.fillWidth: true
			Controls.CheckBox {
				id: enableBox
				checked: Kaidan.customConnectionSettingsEnabled
				Kirigami.FormData.label: qsTr("Enable custom connection settings")
			}

			Controls.SpinBox {
				id: portField
				editable: true
				enabled: enableBox.checked
				Kirigami.FormData.label: qsTr("Port:")
				from: 0
				to: 65535
				value: Kaidan.port
			}

			Controls.TextField {
				id: hostField
				text: Kaidan.host
				enabled: enableBox.checked
				Kirigami.FormData.label: qsTr("Hostname:")
			}
		}
		RowLayout {
			Layout.alignment: Qt.AlignRight

			Controls.Button {
				Layout.alignment: Qt.AlignRight
				text: qsTr("Reset")
				onClicked: {
					Kaidan.customConnectionSettingsEnabled = false
					Kaidan.port = undefined
					Kaidan.host = undefined
				}
			}

			Controls.Button {
				text: qsTr("Apply")
				onClicked: {
					Kaidan.customConnectionSettingsEnabled = enableBox.checked
					Kaidan.port = portField.value
					Kaidan.host = hostField.text
					pageStack.layers.pop()
				}
			}
		}
	}
}
