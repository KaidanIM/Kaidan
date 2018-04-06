import org.kde.kirigami 2.0 as Kirigami
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3

Controls.ToolButton {
	id: button

	property string buttonText
	property string iconSource
	property real scaleFactor: 5

	Layout.fillHeight: true
	Layout.preferredHeight: Kirigami.Units.gridUnit * (scaleFactor + 2)
	Layout.preferredWidth: Kirigami.Units.gridUnit * scaleFactor + Kirigami.Units.smallSpacing * 2

	ColumnLayout {
		Kirigami.Icon {
			id: icon
			Layout.fillHeight: true
			Layout.fillWidth: true
			height: Kirigami.Units.gridUnit * scaleFactor
			width: Kirigami.Units.gridUnit * scaleFactor
			Layout.leftMargin: Kirigami.Units.smallSpacing
			Layout.rightMargin: Kirigami.Units.smallSpacing

			Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

			source: iconSource
		}

		Controls.Label {
			text: buttonText
			Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
		}
	}
}
