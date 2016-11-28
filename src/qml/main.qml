import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ApplicationWindow {
	id: root
	visible: true
	width: 360
	height: 720

	pageStack.initialPage: loginPageComponent
	Component {
		id: rosterPageComponent
		RosterPage {}
	}
	Component {
		id: loginPageComponent
		LoginPage {}
	}

	onClosing: kaidan.mainDisconnect()
}
