import QtQuick 2.5
import QtQuick.Controls 1.2 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ScrollablePage {
	id: page
	title: "Login"
	Layout.fillWidth: true
	implicitWidth: applicationWindow().width

	ColumnLayout {
		width: parent.width
		spacing: Units.smallSpacing

		Controls.Label {
			text: "Your Jabber-ID:"
		}
		Controls.TextField {
			id: jidField
			placeholderText: "example@jabber.example.org"
			Layout.alignment: Qt.AlignHCenter
		}

		Controls.Label {
			text: "Your Password:"
		}
		Controls.TextField {
			id: passField
			placeholderText: "Password"
			echoMode: TextInput.Password
			Layout.alignment: Qt.AlignHCenter
		}

		Controls.Button {
			id: connectButton
			text: "Connect"
			onClicked: {
				connectButton.enabled = false;
				kaidan.mainConnect(jidField.text, passField.text);
			}
		}
		Controls.Label {
			id: statusLabel
			text: "Not connected"
		}
	}

	Component.onCompleted: {
		function goToRoster() {
			statusLabel.text = "Connected";
			//we need to disconnect enableConnectButton to prevent calling it on normal disconnection
			kaidan.connectionStateDisconnected.disconnect(enableConnectButton)
			//open the roster page, loaded in main.qml
			applicationWindow().pageStack.push(rosterPageComponent);
		}

		function enableConnectButton() {
			connectButton.enabled = true
		}

		kaidan.connectionStateConnected.connect(goToRoster)
		kaidan.connectionStateDisconnected.connect(enableConnectButton)
	}
}
