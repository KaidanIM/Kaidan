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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.8 as Kirigami
import EmojiModel 0.1

Popup {
	id: root

	property TextArea textArea
	property alias model: view.model

	ColumnLayout {
		anchors.fill: parent

		GridView {
			id: view

			Layout.fillWidth: true
			Layout.fillHeight: true

			cellWidth: Kirigami.Units.gridUnit * 2.5
			cellHeight: Kirigami.Units.gridUnit * 2.5

			boundsBehavior: Flickable.DragOverBounds

			clip: true

			delegate: ItemDelegate {
				width: Kirigami.Units.gridUnit * 2
				height: Kirigami.Units.gridUnit * 2

				contentItem: Text {
					horizontalAlignment: Text.AlignHCenter
					verticalAlignment: Text.AlignVCenter

					font.pointSize: 20
					text: model.unicode
				}

				hoverEnabled: true
				ToolTip.text: model.shortName
				ToolTip.visible: hovered

				onClicked: {
					GridView.view.model.addFavoriteEmoji(model.index);
					textArea.insert(textArea.cursorPosition, model.unicode)
				}
			}

			ScrollBar.vertical: ScrollBar {}
		}

		Rectangle {
			Layout.fillWidth: true
			Layout.preferredHeight: 2

			color: Kirigami.Theme.highlightColor
		}

		Row {
			Repeater {
				model: ListModel {
					ListElement { label: "🔖"; group: Emoji.Group.Favorites }
					ListElement { label: "😏"; group: Emoji.Group.People }
					ListElement { label: "🌲"; group: Emoji.Group.Nature }
					ListElement { label: "🍛"; group: Emoji.Group.Food }
					ListElement { label: "🚁"; group: Emoji.Group.Activity }
					ListElement { label: "🚅"; group: Emoji.Group.Travel }
					ListElement { label: "💡"; group: Emoji.Group.Objects }
					ListElement { label: "🔣"; group: Emoji.Group.Symbols }
					ListElement { label: "🏁"; group: Emoji.Group.Flags }
					ListElement { label: "🔍"; group: Emoji.Group.Invalid }
				}

				delegate: ItemDelegate {
					width: Kirigami.Units.gridUnit * 1.85
					height: Kirigami.Units.gridUnit * 1.85

					contentItem: Text {
						horizontalAlignment: Text.AlignHCenter
						verticalAlignment: Text.AlignVCenter

						font.pointSize: 20
						text: model.label
					}

					hoverEnabled: true
					ToolTip.text: {
						switch (model.group) {
						case Emoji.Group.Favorites:
							return qsTr('Favorites');
						case Emoji.Group.People:
							return qsTr('People');
						case Emoji.Group.Nature:
							return qsTr('Nature');
						case Emoji.Group.Food:
							return qsTr('Food');
						case Emoji.Group.Activity:
							return qsTr('Activity');
						case Emoji.Group.Travel:
							return qsTr('Travel');
						case Emoji.Group.Objects:
							return qsTr('Objects');
						case Emoji.Group.Symbols:
							return qsTr('Symbols');
						case Emoji.Group.Flags:
							return qsTr('Flags');
						case Emoji.Group.Invalid:
							return qsTr('Search');
						}
					}
					ToolTip.visible: hovered
					highlighted: root.model.group === model.group

					onClicked: root.model.group = model.group
				}
			}
		}

		TextField {
			id: searchField

			Timer {
				id: searchTimer

				interval: 500

				onTriggered: root.model.filter = searchField.text
			}

			Layout.fillWidth: true
			Layout.alignment: Qt.AlignVCenter
			visible: root.model.group === Emoji.Group.Invalid
			placeholderText: qsTr("Search emoji")
			selectByMouse: true
			background: Item {}
			rightPadding: clearButton.width

			ToolButton {
				id: clearButton

				visible: searchField.text !== ''
				icon.name: 'edit-clear'
				focusPolicy: Qt.NoFocus

				anchors {
					verticalCenter: parent.verticalCenter
					right: parent.right
				}

				onClicked: searchField.clear()
			}

			onTextChanged: searchTimer.restart()

			onVisibleChanged: if (visible) forceActiveFocus()
		}
	}
}
