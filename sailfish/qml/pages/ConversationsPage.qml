import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page;

    SilicaListView {
        id: view;
        spacing: Theme.paddingMedium;
        header: PageHeader {
            title: qsTr ("Conversations");
        }
        model: [
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
            { },
        ];
        delegate: BackgroundItem {
            id: item;
            contentHeight: Theme.itemSizeLarge;
            onClicked: { pageStack.push (pageMessaging, { "conversationId" : "123" }); }

            Column {
                anchors {
                    left: parent.left;
                    right: parent.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }

                Label {
                    text: (modelData ["group"] || "Group %1".arg (model.index));
                    color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                }
                Label {
                    text: (modelData ["summary"] || "blah blah blah...");
                    color: Theme.secondaryColor;
                }
            }
        }
        anchors.fill: parent;

        VerticalScrollDecorator { }
    }
}
