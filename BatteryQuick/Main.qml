import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Fusion

Window {
    id: root

    width: 300
    height: 160
    visible: true
    title: qsTr("Battery")

    RowLayout {
        spacing: 10
        anchors.fill: parent

        Battery {
            id: battery

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            value: 20
        }

        Slider {
            id: control

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 10
            orientation: Qt.Vertical
            focus: false
            from: 0
            to: 100
            value: battery.value

            onValueChanged: {
                // console.log("value: " + value)
                battery.value = value
            }

            background: Rectangle {
                x: control.leftPadding + control.availableWidth / 2 - width / 2
                y: control.topPadding
                width: 4
                height: control.availableHeight
                radius: 2
                color: "#21be2b"

                Rectangle {
                    width: parent.width
                    height: control.visualPosition * parent.height
                    color: "#bdbebf"
                    radius: 2
                }
            }

            handle: Rectangle {
                x: control.leftPadding + control.availableWidth / 2 - width / 2
                y: control.topPadding + control.visualPosition * (control.availableHeight - height)
                implicitWidth: 26
                implicitHeight: 26
                radius: 13
                color: control.pressed ? "#f0f0f0" : "#f6f6f6"
                border.color: "#bdbebf"
            }
        }
    }
}
