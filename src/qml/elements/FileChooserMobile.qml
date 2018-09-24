/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2018 Kaidan developers and contributors
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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.0 as Kirigami
import Qt.labs.folderlistmodel 2.1

Item {
	id: root

	property url fileUrl
	property string title
	signal accepted
	property string nameFilter

	Component {
		id: fileChooserPage

		Kirigami.ScrollablePage {
			title: root.title

			actions {
				main: Kirigami.Action {
					id: parentFolderButton
					tooltip: qsTr("go to parent folder")
					iconName: "go-parent-folder"
					onTriggered: fileModel.folder = fileModel.parentFolder
					enabled: fileModel.parentFolder != "file:///"
				}
				right: Kirigami.Action {
					tooltip: qsTr("Close")
					iconName: "dialog-close"
					onTriggered: pageStack.pop()
					enabled: true
				}
			}

			FolderListModel {
				id: fileModel
				nameFilters: root.nameFilter
				showDirsFirst: true
				showDotAndDotDot: false // replaced by the main action Button
				showOnlyReadable: true
			}

			ListView {
				id: view
				model: fileModel
				anchors.fill: parent

				delegate: Kirigami.BasicListItem {
					width: parent.width
					reserveSpaceForIcon: true

					icon: (fileIsDir ? "folder" : "text-x-plain")
					label: fileName + (fileIsDir ? "/" : "")

					onClicked: {
						if (fileIsDir) {
							if (fileName === "..")
								fileModel.folder = fileModel.parentFolder
							else if (fileName === ".")
								return
							else
								fileModel.folder = "file://" + filePath
						} else {
							root.fileUrl = filePath
							root.accepted()
							pageStack.pop()
						}
					}
				}
			}
		}
	}

	function open() {
		pageStack.push(fileChooserPage)
	}
}
