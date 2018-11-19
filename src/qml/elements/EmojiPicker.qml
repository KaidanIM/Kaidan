import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.0 as Kirigami

Popup {
    property var emojiModel
    property var textArea
    property string emojiCategory: "people"

    ColumnLayout {
        anchors.fill: parent

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            cellWidth: Kirigami.Units.gridUnit * 2.5
            cellHeight: Kirigami.Units.gridUnit * 2.5

            boundsBehavior: Flickable.DragOverBounds

            clip: true

            model: emojiModel.model[emojiCategory]

            delegate: ItemDelegate {
                width: Kirigami.Units.gridUnit * 2
                height: Kirigami.Units.gridUnit * 2

                contentItem: Text {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    font.pointSize: 20
                    font.family: "NotoColorEmoji"
                    text: modelData.unicode
                }

                hoverEnabled: true
                ToolTip.text: modelData.shortname
                ToolTip.visible: hovered

                onClicked: textArea.insert(textArea.cursorPosition, modelData.unicode)
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
                    ListElement { label: "😏"; category: "people" }
                    ListElement { label: "🌲"; category: "nature" }
                    ListElement { label: "🍛"; category: "food"}
                    ListElement { label: "🚁"; category: "activity" }
                    ListElement { label: "🚅"; category: "travel" }
                    ListElement { label: "💡"; category: "objects" }
                    ListElement { label: "🔣"; category: "symbols" }
                    ListElement { label: "🏁"; category: "flags" }
                }

                delegate: ItemDelegate {
                    width: Kirigami.Units.gridUnit * 2
                    height: Kirigami.Units.gridUnit * 2

                    contentItem: Text {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.pointSize: 20
                        font.family: "NotoColorEmoji"
                        text: label
                    }

                    hoverEnabled: true
                    ToolTip.text: category
                    ToolTip.visible: hovered

                    onClicked: emojiCategory = category
                }
            }
        }
    }
}
