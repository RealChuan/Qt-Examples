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
    property int scaleMajor: 10
    property int scaleMinor: 5
    property color scaleColor: "#04a8ad"
    property color scaleTextColor: "#908574"

    // === Classic 主题默认颜色 ===
    property color arcColor: "#383d4a"
    property color pointerColor: "#04b5c8"
    property color valueColor: "#908574"
    property color titleColor: "#908574"
    property color backgroundColor: "transparent"

    // === 动画属性 ===
    property int animationDuration: 800
    property bool animationEnabled: true

    // === 比例常量（与 QWidget 版本完全一致）===
    readonly property real arcWidth: minSize / 20.0
    readonly property real arcRadius: minSize * 0.38 - arcWidth / 2
    readonly property real scaleRadius: minSize * 0.40
    readonly property real scaleNumberRadius: minSize * 0.46
    readonly property real majorScaleInLength: minSize / 15.0
    readonly property real majorScaleOutLength: minSize / 30.0
    readonly property real minorScaleLength: minSize / 45.0
    readonly property real pointerLength: minSize * 0.35
    readonly property real pointerHalfWidth: minSize / 100.0
    readonly property real pointerTail: minSize / 30.0
    readonly property real hubRadius: minSize / 40.0
    readonly property real hubInnerRatio: 0.4
    readonly property real valueY: minSize * 0.15
    readonly property real titleY: minSize * 0.25

    // === 计算属性 ===
    readonly property real minSize: Math.min(width, height)
    readonly property real totalAngle: endAngle - startAngle
    readonly property real progress: (value - minValue) / (maxValue - minValue)
    readonly property real currentAngle: startAngle + progress * totalAngle
    readonly property real centerX: width / 2
    readonly property real centerY: height / 2
    readonly property real progressSweep: totalAngle * progress

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

    onValueChanged: {
        if (!privateData.isAnimating) {
            if (value > privateData.previousValue) {
                root.valueIncreased(value);
            } else if (value < privateData.previousValue) {
                root.valueDecreased(value);
            }
            privateData.previousValue = value;
        }
    }

    // === 背景 ===
    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    // === 弧线（背景轨道 + 进度弧，FlatCap 锐利切口）===
    Shape {
        anchors.fill: parent
        vendorExtensionsEnabled: true
        preferredRendererType: Shape.CurveRenderer

        // 背景轨道弧
        ShapePath {
            strokeColor: root.arcColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.FlatCap

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.startAngle
                sweepAngle: root.totalAngle
            }
        }

        // 进度弧
        ShapePath {
            strokeColor: root.valueColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.FlatCap

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.startAngle
                sweepAngle: root.progressSweep
            }
        }
    }

    // === 刻度线（主刻度双向延伸，次刻度单向向内）===
    Repeater {
        model: root.scaleMajor * root.scaleMinor + 1

        Rectangle {
            id: scale
            required property int index
            readonly property real angle: root.startAngle + index * (root.totalAngle / (root.scaleMajor * root.scaleMinor))
            readonly property bool isMajor: index % root.scaleMinor === 0
            readonly property real inLen: isMajor ? root.majorScaleInLength : root.minorScaleLength
            readonly property real outLen: isMajor ? root.majorScaleOutLength : 0
            readonly property real totalLen: inLen + outLen
            readonly property real lineW: isMajor ? 2 : 1
            readonly property real midR: root.scaleRadius + (outLen - inLen) / 2
            readonly property real rad: scale.angle * Math.PI / 180

            width: lineW
            height: totalLen
            x: root.centerX + midR * Math.cos(rad) - lineW / 2
            y: root.centerY + midR * Math.sin(rad) - totalLen / 2
            rotation: scale.angle + 90
            transformOrigin: Item.Center
            color: isMajor ? root.scaleColor : Qt.rgba(root.scaleColor.r, root.scaleColor.g, root.scaleColor.b, 0.39)
            antialiasing: true
        }
    }

    // === 刻度数字 ===
    Repeater {
        model: root.scaleMajor + 1

        Text {
            required property int index
            readonly property real angle: root.startAngle + index * (root.totalAngle / root.scaleMajor)
            readonly property real numberValue: root.minValue + index * (root.maxValue - root.minValue) / root.scaleMajor
            readonly property real rad: angle * Math.PI / 180

            text: numberValue.toFixed(0)
            color: root.scaleTextColor
            font {
                family: "Consolas"
                pixelSize: root.minSize / 26.0
                bold: false
            }
            antialiasing: true

            x: root.centerX + root.scaleNumberRadius * Math.cos(rad) - width / 2
            y: root.centerY + root.scaleNumberRadius * Math.sin(rad) - height / 2
        }
    }

    // === 指针（细长剑形：长端指向弧方向，短尾部）===
    Shape {
        id: pointer
        x: root.centerX
        y: root.centerY
        width: 1
        height: 1
        rotation: root.currentAngle - 90
        transformOrigin: Item.TopLeft
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: root.pointerColor
            fillColor: root.pointerColor
            strokeWidth: 0
            joinStyle: ShapePath.MiterJoin

            PathMove {
                x: -root.pointerHalfWidth
                y: 0
            }
            PathLine {
                x: 0
                y: root.pointerLength
            }
            PathLine {
                x: root.pointerHalfWidth
                y: 0
            }
            PathLine {
                x: 0
                y: -root.pointerTail
            }
        }
    }

    // === 中心轴（六边形外环 + 圆形内核）===
    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        // 六边形外环
        ShapePath {
            strokeColor: "transparent"
            fillColor: root.pointerColor
            strokeWidth: 0

            PathMove {
                x: root.centerX
                y: root.centerY - root.hubRadius
            }
            PathLine {
                x: root.centerX + root.hubRadius * 0.866
                y: root.centerY - root.hubRadius * 0.5
            }
            PathLine {
                x: root.centerX + root.hubRadius * 0.866
                y: root.centerY + root.hubRadius * 0.5
            }
            PathLine {
                x: root.centerX
                y: root.centerY + root.hubRadius
            }
            PathLine {
                x: root.centerX - root.hubRadius * 0.866
                y: root.centerY + root.hubRadius * 0.5
            }
            PathLine {
                x: root.centerX - root.hubRadius * 0.866
                y: root.centerY - root.hubRadius * 0.5
            }
        }

        // 圆形内核
        ShapePath {
            strokeColor: "transparent"
            fillColor: root.valueColor
            strokeWidth: 0

            PathAngleArc {
                centerX: root.centerX
                centerY: root.centerY
                radiusX: root.hubRadius * root.hubInnerRatio
                radiusY: root.hubRadius * root.hubInnerRatio
                startAngle: 0
                sweepAngle: 360
            }
        }
    }

    // === 数值显示（与 Widget 版本对齐：中心向下 0.15*minSize）===
    Text {
        id: valueText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: root.valueY
        }
        text: root.value.toFixed(2) + " " + root.unit
        color: root.valueColor
        font {
            family: "Consolas"
            pixelSize: root.minSize / 14.0
            bold: true
        }
        horizontalAlignment: Text.AlignHCenter
    }

    // === 标题（与 Widget 版本对齐：中心向下 0.25*minSize）===
    Text {
        id: titleText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: root.titleY
        }
        text: root.title.toUpperCase()
        color: root.titleColor
        font {
            family: "Consolas"
            pixelSize: root.minSize / 30.0
            bold: false
            letterSpacing: 2
        }
        horizontalAlignment: Text.AlignHCenter
    }

    // === 公共方法 ===
    function setValue(newValue: real) {
        const clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            root.animationEnabled = false;
            root.value = clampedValue;
            root.animationEnabled = true;
        }
    }

    function setValueAnimated(newValue: real) {
        const clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            root.value = clampedValue;
        }
    }

    function increaseValue(increment: real) {
        if (increment > 0) {
            root.setValueAnimated(root.value + increment);
        }
    }

    function decreaseValue(decrement: real) {
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

    Component.onCompleted: {
        privateData.previousValue = root.value;
    }
}
