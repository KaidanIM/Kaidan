import QtQuick 2.0
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0

Page {
    id: page;

    property string conversationId : "";

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png";
        source: "image://glass/file:///tmp/tst.png";
        opacity: 0.85;
        sourceSize: Qt.size (Screen.width, Screen.height);
        asynchronous: false
        anchors.centerIn: parent;
    }
    Item {
        id: banner;
        height: Theme.itemSizeLarge;
        anchors {
            top: parent.top;
            left: parent.left;
            right: parent.right;
        }

        Rectangle {
            z: -1;
            color: "black";
            opacity: 0.15;
            anchors.fill: parent;
        }
        Image {
            id: avatar;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: "qrc:///qml/img/avatar.png";
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
                right: parent.right;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Rectangle {
                z: -1;
                color: "black";
                opacity: 0.35;
                anchors.fill: parent;
            }
        }
        Column {
            anchors {
                right: avatar.left;
                margins: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }

            Label {
                text:  conversationId;
                color: Theme.highlightColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeLarge;
                }
                anchors.right: parent.right;
            }
            Label {
                text: qsTr ("last seen yesterday, 12:30 PM");
                color: Theme.secondaryColor;
                font {
                    family: Theme.fontFamilyHeading;
                    pixelSize: Theme.fontSizeTiny;
                }
                anchors.right: parent.right;
            }
        }
    }
    SilicaListView {
        id: view;
        clip: true;
        model: kaidan.persistence.messageController
        
        header: Item {
            height: view.spacing;
            anchors {
                left: parent.left;
                right: parent.right;
            }
        }
        footer: Item {
            height: view.spacing;
            anchors {
                left: parent.left;
                right: parent.right;
            }
        }
        spacing: Theme.paddingMedium;
        delegate: Item {
            id: item;
            height: shadow.height;
            anchors {
                left: parent.left;
                right: parent.right;
                margins: view.spacing;
            }

            readonly property bool alignRight      : (direction == 1);
            readonly property int  maxContentWidth : (width * 0.85);

            Rectangle {
                id: shadow;
                color: "white";
                radius: 3;
                opacity: (item.alignRight ? 0.05 : 0.15);
                antialiasing: true;
                anchors {
                    fill: layout;
                    margins: -Theme.paddingSmall;
                }
            }
            Column {
                id: layout;
                anchors {
                    left: (item.alignRight ? parent.left : undefined);
                    right: (!item.alignRight ? parent.right : undefined);
                    margins: -shadow.anchors.margins;
                    verticalCenter: parent.verticalCenter;
                }

                Text {
                    text: message;
                    color: Theme.primaryColor;
                    width: Math.min (item.maxContentWidth, contentWidth);
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                    visible: (modelData ["type"] === "text");
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeMedium;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Image {
                    source: (visible ? modelData ["content"] || "" : "");
                    width: Math.min (item.maxContentWidth, sourceSize.width);
                    fillMode: Image.PreserveAspectFit;
                    visible: (modelData ["type"] === "img");
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
                Label {
                    text: received;
                    color: Theme.secondaryColor;
                    font {
                        family: Theme.fontFamilyHeading;
                        pixelSize: Theme.fontSizeTiny;
                    }
                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                    }
                }
            }
        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: sendmsgview.top;
        }
    }

    Row {
        id: sendmsgview
        anchors {
            left: parent.left;
            right: parent.right;
            bottom: parent.bottom;
        }

        TextArea {
            id: editbox;
            placeholderText: qsTr ("Enter message...");
            width: parent.width - 100
        }
        IconButton {
            id: sendButton
            icon.source: "image://theme/icon-m-enter-accept"
            width: 100
            onClicked: {
                var msgToSend = editbox.text;
                editbox.text = " ";

                kaidan.sendMessage(conversationId, msgToSend);
            }
        }
    }
}
