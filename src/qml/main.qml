import QtQuick 2.0
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ApplicationWindow {
	id: root
	visible: true

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
