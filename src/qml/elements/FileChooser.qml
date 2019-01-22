/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

import QtQuick 2.6
import org.kde.kirigami 2.0 as Kirigami

Item {
	id: root

	property string filter: "*"
	property string filterName: "All files"
	property string fileUrl
	property bool selectFolder: false
	property string title: qsTr("Select a file")
	signal accepted

	Loader {
		id: fileChooserLoader
	}

	function open() {
		fileChooserLoader.item.open()
	}

	Component.onCompleted: {
		if (Kirigami.Settings.isMobile) {
			fileChooserLoader.setSource("FileChooserMobile.qml",
			{
				"nameFilter": filter,
				"title": title
			})
		}
		else if (!Kirigami.Settings.isMobile) {
			var selectedNameFilter = filterName + " (" + filter + ")"
			fileChooserLoader.setSource("FileChooserDesktop.qml",
			{
				"selectedNameFilter": selectedNameFilter,
				"selectFolder": selectFolder,
				"title": title
			})
		}
		else {
			fileChooserLoader.setSource("FileChooserDesktop.qml")
		}
	}

	Connections {
		target: fileChooserLoader.item
		onAccepted: {
			fileUrl = fileChooserLoader.item.file
			root.accepted()
			console.log("Child file dialog accepted. URL: " + fileUrl)
		}
	}
}
