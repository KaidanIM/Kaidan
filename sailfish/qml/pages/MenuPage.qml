import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingLarge;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: "Mitakuuluu";
            }
            Image {
                id: img;
                source: "qrc:///qml/img/avatar.png";
                width: Theme.iconSizeLarge;
                height: width;
                anchors.horizontalCenter: parent.horizontalCenter;
            }
            Label {
                text: "Joe Mc Testa";
                font.pixelSize: Theme.fontSizeLarge;
                anchors {
                    left: parent.left;
                    margins: Theme.paddingMedium;
                }
            }
            ComboBox {
                label: qsTr ("Status");
                currentIndex: 0;
                menu: ContextMenu {
                    MenuItem { text: "Online"; }
                    MenuItem { text: "Away"; }
                    MenuItem { text: "Offline"; }
                }
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
            Separator {
                primaryColor: Qt.rgba (1,1,1, 0.5);
                secondaryColor: Qt.rgba (1,1,1, 0.0);
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
        }
        model: [
            { "title" : qsTr ("Contacts"),      "icon" : "image://theme/icon-m-people",         "page" : pageContacts },
            { "title" : qsTr ("Conversations"), "icon" : "image://theme/icon-m-chat",           "page" : pageConversations },
            { "title" : qsTr ("Preferences"),   "icon" : "image://theme/icon-l-developer-mode", "page" : pagePreferences },
            { "title" : qsTr ("Account"),       "icon" : "image://theme/icon-s-secure",         "page" : pageAccount },
        ];
        delegate: BackgroundItem {
            id: item;
            height: Theme.itemSizeMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }
            onClicked: {
                pageStack.push (modelData ["page"]);
                pageStack.navigateForward ();
            }

            Image {
                id: img;
                source: modelData ["icon"];
                width: Theme.iconSizeMedium;
                height: width;
                anchors {
                    left: parent.left;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }
            }
            Label {
                id: lbl;
                text: modelData ["title"];
                anchors {
                    left: img.right;
                    margins: Theme.paddingLarge;
                    verticalCenter: parent.verticalCenter;
                }
            }
        }
        footer: Column {
            spacing: Theme.paddingLarge;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            Separator {
                primaryColor: Qt.rgba (1,1,1, 0.5);
                secondaryColor: Qt.rgba (1,1,1, 0.0);
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
            }
            Label {
                text: qsTr ("Messaging client for SailfishOS");
                color: Theme.secondaryColor;
                font.pixelSize: Theme.fontSizeSmall;
                anchors {
                    left: parent.left;
                    margins: Theme.paddingMedium;
                }
            }
        }
        anchors.fill: parent

        VerticalScrollDecorator {}
    }
}
