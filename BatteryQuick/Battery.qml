import QtQuick

Rectangle {
    id: root

    width: 200
    height: 100

    property color borderColor: "#505008"
    property color powerColor: "#41cd52"
    property color alarmColor: "#fa7671"
    property int alarmValue: 20
    property int value: 0

    Rectangle {
        id: batteryRect

        x: 10
        y: 10
        width: root.width - x * 3
        height: root.height - y * 2
        border.color: root.borderColor
        border.width: 2
        radius: 5

        Rectangle {
            x: batteryRect.border.width * 2
            y: batteryRect.border.width * 2
            width: (batteryRect.width - x * 2) * (value / 100)
            height: batteryRect.height - y * 2
            color: value > root.alarmValue ? root.powerColor : root.alarmColor
            radius: batteryRect.radius
        }

        Text {
            text: value.toString() + "%"
            font.pointSize: {
                var width = batteryRect.width / 5
                return width > 0 ? width : 14
            }
            color: value > root.alarmValue ? "black" : root.alarmColor
            anchors.centerIn: batteryRect
        }
    }

    Rectangle {
        width: 10
        height: 20
        color: root.borderColor
        x: batteryRect.x + batteryRect.width
        y: (root.height - height) / 2
    }
}
