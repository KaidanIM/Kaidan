import QtQuick 2.1
import QtQuick.Controls 1.4
import harbour.kaidan 1.0

ApplicationWindow{
	visible: true
	width: 360
	height: 720

	Flickable{
		anchors.fill: parent

		ListView {
			anchors.fill: parent
			model: kaidan.rosterController.rosterList
			delegate: Rectangle {
				height: 25
				width: parent.width
				Text { text: model.modelData.jid}
			}
		}
	}
	onClosing: kaidan.mainQuit()
}


