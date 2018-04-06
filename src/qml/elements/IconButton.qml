import org.kde.kirigami 2.0 as Kirigami
import QtQuick.Controls 2.0 as Controls
import QtQuick.Layouts 1.3

Controls.ToolButton {
	property string buttonText
	property string imageSource

	implicitHeight: buttonContent.height + Kirigami.Units.gridUnit

	ColumnLayout {
		id: buttonContent

		Kirigami.Icon {
			height: 50
			width: height

			source: imageSource
		}
		Controls.Label {
			text: buttonText
		}
	}
}
