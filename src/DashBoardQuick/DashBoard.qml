pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Item {
    id: root

    // === 仪表盘核心属性 ===
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property real startAngle: 130
    property real endAngle: 410
    property string unit: "km/h"
    property string title: "SPEED"

    // === 刻度属性 ===
    property int scaleMajor: 10      // 主刻度数量
    property int scaleMinor: 5       // 每个主刻度间的次刻度数量
    property color scaleColor: "#04A8AD"
    property color scaleTextColor: "#04A8AD"

    // === 颜色属性 ===
    property color arcColor: "#383D4A"       // 背景弧颜色
    property color pointerColor: "#04B5C8"    // 指针颜色
    property color valueColor: "#2c3e50"      // 数值颜色
    property color titleColor: "#7f8c8d"      // 标题颜色
    property color backgroundColor: "transparent"

    // === 尺寸属性 ===
    property real arcWidth: Math.min(width, height) * 0.08
    property real pointerWidth: Math.min(width, height) * 0.02
    property real pointerLength: Math.min(width, height) * 0.45

    // === 动画属性 ===
    property int animationDuration: 800
    property bool animationEnabled: true

    // === 计算属性 ===
    readonly property real totalAngle: endAngle - startAngle
    readonly property real progress: (value - minValue) / (maxValue - minValue)
    readonly property real currentAngle: startAngle + progress * totalAngle
    readonly property real centerX: width / 2
    readonly property real centerY: height / 2
    readonly property real radius: Math.min(width, height) / 2 - arcWidth / 2

    // === 信号 ===
    signal valueIncreased(real newValue)
    signal valueDecreased(real newValue)
    signal valueReset
    signal animationStarted(real oldValue, real newValue)
    signal animationFinished(real value)

    // === 私有数据 ===
    QtObject {
        id: privateData
        property real previousValue: root.minValue
        property bool isAnimating: false
    }

    // === 隐式尺寸 ===
    implicitWidth: 200
    implicitHeight: 200

    // === 动画系统 ===
    Behavior on value {
        id: valueBehavior
        enabled: root.animationEnabled && root.animationDuration > 0
        NumberAnimation {
            duration: root.animationDuration
            easing.type: Easing.OutCubic

            onStarted: {
                privateData.isAnimating = true;
                root.animationStarted(privateData.previousValue, root.value);
            }

            onStopped: {
                privateData.isAnimating = false;
                privateData.previousValue = root.value;
                root.animationFinished(root.value);
            }
        }
    }

    // === 值变化处理 ===
    onValueChanged: {
        if (!privateData.isAnimating) {
            privateData.previousValue = value;

            // 直接设置值，检查增减
            if (value > privateData.previousValue) {
                root.valueIncreased(value);
            } else if (value < privateData.previousValue) {
                root.valueDecreased(value);
            }
        }
    }

    // === 外观组件 ===

    // 背景
    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    // 主形状容器
    Shape {
        id: progressShape
        anchors.fill: parent
        vendorExtensionsEnabled: true
        preferredRendererType: Shape.CurveRenderer

        // 背景圆弧
        ShapePath {
            id: baseArc
            strokeColor: root.arcColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.radius
                radiusY: root.radius
                startAngle: root.startAngle
                sweepAngle: root.totalAngle
            }
        }
    }

    // 刻度线
    Repeater {
        model: root.scaleMajor * root.scaleMinor + 1

        Rectangle {
            id: scale
            required property int index
            readonly property real angle: root.startAngle + index * (root.totalAngle / (root.scaleMajor * root.scaleMinor))
            readonly property bool isMajor: index % root.scaleMinor === 0
            readonly property real length: isMajor ? root.radius * 0.12 : root.radius * 0.06
            readonly property real lineWidth: isMajor ? 2 : 1

            width: lineWidth
            height: length
            color: root.scaleColor
            antialiasing: true

            transform: [
                Rotation {
                    origin.x: scale.lineWidth / 2
                    origin.y: 0
                    angle: scale.angle - 90
                },
                Translate {
                    x: root.centerX + (root.radius - scale.length) * Math.cos(scale.angle * Math.PI / 180) - scale.lineWidth / 2
                    y: root.centerY + (root.radius - scale.length) * Math.sin(scale.angle * Math.PI / 180)
                }
            ]
        }
    }

    // 刻度数字
    Repeater {
        model: root.scaleMajor + 1

        Text {
            required property int index
            readonly property real angle: root.startAngle + index * (root.totalAngle / root.scaleMajor)
            readonly property real numberValue: root.minValue + index * (root.maxValue - root.minValue) / root.scaleMajor

            text: numberValue.toFixed(1)
            color: root.scaleTextColor
            font.pixelSize: Math.min(root.width, root.height) * 0.05
            antialiasing: true

            x: root.centerX + (root.radius * 0.75) * Math.cos(angle * Math.PI / 180) - width / 2
            y: root.centerY + (root.radius * 0.75) * Math.sin(angle * Math.PI / 180) - height / 2
        }
    }

    // 指针
    Shape {
        id: pointer
        x: root.centerX
        y: root.centerY
        width: root.pointerWidth
        height: root.pointerLength
        preferredRendererType: Shape.CurveRenderer

        // 指针形状 - 简单的三角形，指向正下方
        ShapePath {
            strokeColor: root.pointerColor
            fillColor: root.pointerColor
            strokeWidth: 0
            joinStyle: ShapePath.MiterJoin

            PathMove {
                x: 0
                y: 0
            }
            PathLine {
                x: -root.pointerWidth
                y: root.radius / 20
            }
            PathLine {
                x: root.pointerWidth
                y: root.pointerLength
            }
            PathLine {
                x: root.pointerWidth
                y: root.radius / 20
            }
            PathLine {
                x: 0
                y: 0
            }
        }

        rotation: root.currentAngle - 90
        transformOrigin: Item.TopLeft
    }

    // 数值显示
    Text {
        id: valueText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: -parent.height * 0.1
        }
        text: root.value.toFixed(2) + " " + root.unit
        color: root.valueColor
        font {
            pixelSize: Math.min(root.width, root.height) * 0.1
            bold: true
            family: "Arial"
        }
        horizontalAlignment: Text.AlignHCenter
    }

    // 标题
    Text {
        id: titleText
        anchors {
            top: valueText.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: Math.min(root.width, root.height) * 0.05
        }
        text: root.title
        color: root.titleColor
        font {
            pixelSize: Math.min(root.width, root.height) * 0.06
            capitalization: Font.AllUppercase
            letterSpacing: 1.5
        }
        horizontalAlignment: Text.AlignHCenter
    }

    // === 公共方法 ===
    function setValue(newValue) {
        var clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            root.animationEnabled = false;
            root.value = clampedValue;
            root.animationEnabled = true;
        }
    }

    function setValueAnimated(newValue) {
        var clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            root.value = clampedValue;
        }
    }

    function increaseValue(increment = 1.0) {
        if (increment > 0) {
            root.setValueAnimated(root.value + increment);
        }
    }

    function decreaseValue(decrement = 1.0) {
        if (decrement > 0) {
            root.setValueAnimated(root.value - decrement);
        }
    }

    function reset() {
        if (root.value !== root.minValue) {
            root.setValueAnimated(root.minValue);
            root.valueReset();
        }
    }

    // === 组件初始化 ===
    Component.onCompleted: {
        privateData.previousValue = root.value;
    }
}
