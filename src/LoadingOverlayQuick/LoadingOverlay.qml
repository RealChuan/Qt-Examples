import QtQuick
import QtQuick.Controls

Popup {
    id: root

    property string text: ""  // 加载文本
    property color textColor: "#04B5C8"

    anchors.centerIn: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    background: Rectangle {
        color: "transparent"
    }

    contentItem: Column {
        spacing: 10
        anchors.centerIn: parent

        BusyIndicator {
            running: root.visible
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            text: root.text
            color: root.textColor
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            visible: text !== ""
        }
    }

    function show(message) {
        if (message !== undefined) {
            text = message;
        }
        open();
    }

    function hide() {
        close();
    }
}
