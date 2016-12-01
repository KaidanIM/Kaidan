import QtQuick 2.0
import QtQuick.Controls 1.2 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ScrollablePage {
	id: page
	title: "Login"
	Layout.fillWidth: true
	implicitWidth: applicationWindow().width

	GridLayout {
		columns: 2
		anchors.fill: parent

		Layout.fillWidth: true
		Controls.Label {
			text: "Your Jabber-ID:"
		}
		Controls.TextField {
			id: jidField
			placeholderText: "user@example.org"
			Layout.fillWidth: true
		}

		Controls.Label {
			text: "Your Password:"
		}
		Controls.TextField {
			id: passField
			placeholderText: "Password"
			echoMode: TextInput.Password
			Layout.fillWidth: true
		}

		Controls.Button {
			id: connectButton
			text: "Connect"
			Layout.columnSpan: 2
			Layout.alignment: Qt.AlignRight
			onClicked: {
				connectButton.enabled = false;
				connectButton.text = "<i>Connecting...</i>"
				kaidan.mainConnect(jidField.text, passField.text);
			}
		}
	}

	Component.onCompleted: {
		function goToRoster() {
			//we need to disconnect enableConnectButton to prevent calling it on normal disconnection
			kaidan.connectionStateDisconnected.disconnect(enableConnectButton)
			//open the roster page, loaded in main.qml
			applicationWindow().pageStack.replace(rosterPageComponent)
		}

		function enableConnectButton() {
			connectButton.text = "Retry"
			connectButton.enabled = true
		}

		kaidan.connectionStateConnected.connect(goToRoster)
		kaidan.connectionStateDisconnected.connect(enableConnectButton)
	}
}
