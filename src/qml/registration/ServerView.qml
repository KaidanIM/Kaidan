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
import org.kde.kirigami 2.8 as Kirigami

import im.kaidan.kaidan 1.0

import "../elements"

/**
 * This view is used for choosing a server.
 */
View {
	descriptionText: qsTr("The server is the provider your communication will be managed by.\nThe selectable servers are hand-picked by our community!")
	imageSource: "server"

	property string text: customServerSelected ? customServerField.text : serverListModel.data(comboBox.currentIndex, ServerListModel.JidRole)
	property bool customServerSelected: serverListModel.data(comboBox.currentIndex, ServerListModel.IsCustomServerRole)
	property bool inBandRegistrationSupported: serverListModel.data(comboBox.currentIndex, ServerListModel.SupportsInBandRegistrationRole)
	property string registrationWebPage: serverListModel.data(comboBox.currentIndex, ServerListModel.RegistrationWebPageRole)
	property bool shouldWebRegistrationViewBeShown: !customServerSelected && !inBandRegistrationSupported

	ColumnLayout {
		parent: contentArea
		spacing: Kirigami.Units.largeSpacing

		Controls.Label {
			text: qsTr("Server")
		}

		Controls.ComboBox {
			id: comboBox
			Layout.fillWidth: true
			model: ServerListModel {
				id: serverListModel
			}
			textRole: "display"
			currentIndex: indexOfRandomlySelectedServer()
			onCurrentIndexChanged: customServerField.text = ""

			onActivated: {
				if (index === 0) {
					editText = ""

					// Focus the whole combo box.
					forceActiveFocus()

					// Focus the input text field of the combo box.
					nextItemInFocusChain().forceActiveFocus()
				}
			}
		}

		Controls.TextField {
			id: customServerField
			visible: customServerSelected
			Layout.fillWidth: true
			placeholderText: "example.org"
		}

		Controls.ScrollView {
			Layout.fillWidth: true
			Layout.fillHeight: true

			Kirigami.FormLayout {
				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Web registration only:")
					text: inBandRegistrationSupported ? qsTr("No") : qsTr("Yes")
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Region:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.CountryRole)
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Server language:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.LanguageRole)
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Server website:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.WebsiteRole)
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Online since:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.OnlineSinceRole)
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Maximum size for sending files:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.HttpUploadSizeRole)
				}

				Controls.Label {
					visible: !customServerSelected && text
					Kirigami.FormData.label: qsTr("Duration of message storage:")
					text: serverListModel.data(comboBox.currentIndex, ServerListModel.MessageStorageDurationRole)
				}
			}
		}

		// placeholder
		Item {
			Layout.fillHeight: true
		}
	}

	onShouldWebRegistrationViewBeShownChanged: {
		// Show the web registration view for non-custom servers if only web registration is supported or hides the view otherwise.
		if (shouldWebRegistrationViewBeShown)
			addWebRegistrationView()
		else
			removeWebRegistrationView()
	}

	/**
	 * Randomly sets a new server as selected for registration.
	 */
	function selectServerRandomly() {
		comboBox.currentIndex = indexOfRandomlySelectedServer()
	}

	/**
	 * Returns the index of a randomly selected server for registration.
	 */
	function indexOfRandomlySelectedServer() {
		return serverListModel.randomlyChooseIndex()
	}
}
