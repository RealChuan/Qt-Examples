pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: root

    // 属性定义 (iOS 风格调色板)
    property color borderColor: "#d1d1d6"
    property color backgroundColor: "#f2f2f7"
    property color hourColor: "#1c1c1e"
    property color minuteColor: "#3a3a3c"
    property color secondColor: "#ff3b30"
    property color textColor: "#1c1c1e"
    property bool smoothSeconds: true
    property bool showSeconds: true

    // 内部计算属性
    readonly property real centerX: width / 2
    readonly property real centerY: height / 2
    readonly property real radius: Math.min(centerX, centerY) * 0.9
    readonly property real borderWidth: radius / 30
    readonly property real scaleLength: radius / 15
    readonly property real numberRadius: radius * 0.75

    // 当前时间
    property date currentTime: new Date()

    // 定时更新
    Timer {
        interval: root.smoothSeconds ? 50 : 1000
        running: true
        repeat: true
        onTriggered: root.currentTime = new Date()
    }

    // 背景圆 (纯色, QML 不支持 ShapeRadialGradient)
    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: "transparent"
            fillColor: root.backgroundColor
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.radius
                radiusY: root.radius
                startAngle: 0
                sweepAngle: 360
            }
        }

        // 边框环 (细薄, 半透明)
        ShapePath {
            strokeWidth: root.borderWidth
            strokeColor: root.borderColor
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.radius - root.borderWidth / 2
                radiusY: root.radius - root.borderWidth / 2
                startAngle: 0
                sweepAngle: 360
            }
        }
    }

    // 刻度 (小时刻度: 深色粗线, 分钟刻度: 浅色细线)
    Repeater {
        model: 60

        Rectangle {
            required property int index
            readonly property bool isHour: index % 5 === 0
            readonly property real angle: index * 6 * Math.PI / 180

            x: root.centerX + (root.radius - root.scaleLength) * Math.cos(angle) - width / 2
            y: root.centerY + (root.radius - root.scaleLength) * Math.sin(angle) - height / 2

            width: isHour ? root.radius * 0.025 : root.radius * 0.008
            height: root.scaleLength * (isHour ? 1.0 : 0.7)
            color: isHour ? root.textColor : "#c7c7cc"
            radius: width / 2

            rotation: index * 6 + 90
            transformOrigin: Item.Bottom
            antialiasing: true
        }
    }

    // 数字
    Repeater {
        model: 12

        Text {
            required property int index
            readonly property real angle: (index - 3) * 30 * Math.PI / 180
            readonly property real xPos: root.centerX + root.numberRadius * Math.cos(angle)
            readonly property real yPos: root.centerY + root.numberRadius * Math.sin(angle)

            x: xPos - width / 2
            y: yPos - height / 2
            text: index + 1
            color: root.textColor
            font.pixelSize: Math.max(10, root.radius * 0.1)
            font.bold: true
            antialiasing: true
        }
    }

    // 时针
    Hand {
        centerX: root.centerX
        centerY: root.centerY
        length: root.radius * 0.5
        handWidth: root.radius * 0.01
        color: root.hourColor
        tailRatio: 0.1
        angle: (root.currentTime.getHours() % 12 + root.currentTime.getMinutes() / 60) * 30
    }

    // 分针
    Hand {
        centerX: root.centerX
        centerY: root.centerY
        length: root.radius * 0.7
        handWidth: root.radius * 0.008
        color: root.minuteColor
        tailRatio: 0.1
        angle: (root.currentTime.getMinutes() + root.currentTime.getSeconds() / 60) * 6
    }

    // 秒针 (较长尾端配重)
    Hand {
        centerX: root.centerX
        centerY: root.centerY
        length: root.radius * 0.8
        handWidth: root.radius * 0.004
        color: root.secondColor
        tailRatio: 0.2
        angle: root.currentTime.getSeconds() * 6 + (root.smoothSeconds ? root.currentTime.getMilliseconds() / 1000 * 6 : 0)
        visible: root.showSeconds
    }

    // 中心点
    Rectangle {
        width: root.radius / 20
        height: width
        x: root.centerX - width / 2
        y: root.centerY - height / 2
        radius: width / 2
        color: root.hourColor
    }
}
