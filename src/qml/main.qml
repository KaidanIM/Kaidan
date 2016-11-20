import QtQuick 2.1
import QtQuick.Controls 1.4
import harbour.kaidan 1.0

ApplicationWindow {
	visible: true
	width: 360
	height: 720

	Loader {
		id: mainLoader
		anchors.fill: parent
		source: "LoginPage.qml"
	}

	onClosing: kaidan.mainDisconnect()
}


