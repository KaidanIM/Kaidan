import QtQuick 2.1
import harbour.kaidan 1.0

Rectangle {
    visible: true
    width: 360
    height: 360

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    Text {
        text: qsTr("Hello World")
        anchors.centerIn: parent
    }

    ListView {
        width: 100; height: 100

        model: kaidan.rosterController.rosterList
        delegate: Rectangle {
            height: 25
            width: 100
            Text { text: model.modelData.jid}
        }
    }
}


