import QtQuick 2.0
import org.kde.kirigami 1.0 as Kirigami
import harbour.kaidan 1.0

Kirigami.ScrollablePage {
	id: rosterPage
	title: "Contacts"

	ListView {
		id: rosterView

		model: kaidan.rosterController.rosterList
		delegate: Kirigami.BasicListItem {
			label: model.modelData.jid
		}
	}
}
