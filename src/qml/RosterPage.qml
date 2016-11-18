import QtQuick 2.1
import harbour.kaidan 1.0

Flickable{
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
