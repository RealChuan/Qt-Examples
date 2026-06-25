import QtQuick
import QtQuick.Shapes

Item {
    id: root

    // === 公共属性 ===
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property real startAngle: 130
    property real endAngle: 410
    property bool showPercent: true
    property string title: "Progress"

    // 颜色属性 (iOS 风格调色板)
    property color arcColor: "#007aff"
    property color textColor: "#1c1c1e"
    property color titleColor: "#8e8e93"
    property color baseColor: "#d1d1d6"
    property color backgroundColor: "transparent"

    // 动画属性
    property int animationDuration: 500

    // 尺寸属性 (与 QWidget 版本保持一致的视觉比例)
    property real arcWidth: Math.min(width, height) * 0.1
    property real arcMargin: Math.min(width, height) * 0.05

    // === 计算属性 ===
    readonly property bool isAnimating: privateData.animationRunning
    readonly property real progress: (value - minValue) / (maxValue - minValue)
    readonly property real totalAngle: endAngle - startAngle
    readonly property real progressAngle: totalAngle * progress
    // 圆弧半径 (与 QWidget 版本一致: minSize/2 - arcWidth - arcMargin)
    readonly property real arcRadius: Math.min(width, height) / 2 - arcWidth - arcMargin

    // === 信号定义 === (只定义QML不会自动生成的信号)
    signal valueIncreased(real newValue)
    signal valueDecreased(real newValue)
    signal valueReset
    signal animationStarted(real oldValue, real newValue)
    signal animationFinished(real value)

    // === 私有数据 ===
    QtObject {
        id: privateData
        property bool animationRunning: false
        property real previousValue: root.value
        property bool updatingFromExternal: false
    }

    // === 隐式尺寸 ===
    implicitWidth: 200
    implicitHeight: 200

    // === 动画系统 ===
    Behavior on value {
        id: valueAnimation
        enabled: root.animationDuration > 0 && privateData.animationRunning
        NumberAnimation {
            duration: root.animationDuration
            easing.type: Easing.OutCubic

            onStarted: {
                root.animationStarted(privateData.previousValue, root.value);
            }

            onStopped: {
                privateData.animationRunning = false;
                root.animationFinished(root.value);
            }
        }
    }

    // === 值变化处理 ===
    onValueChanged: {
        if (!privateData.updatingFromExternal) {
            if (value > privateData.previousValue) {
                valueIncreased(value);
            } else if (value < privateData.previousValue) {
                valueDecreased(value);
            }
            privateData.previousValue = value;
        }
    }

    // === 外观组件 ===

    // 背景卡片 (iOS 风格圆角卡片, 与 QWidget 版本一致)
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: root.backgroundColor
        radius: Math.min(width, height) * 0.1
    }

    // 使用 Shape 绘制圆弧
    Shape {
        id: progressShape
        anchors.fill: parent
        vendorExtensionsEnabled: true
        preferredRendererType: Shape.CurveRenderer

        // 1. 轨道弧 (纯色, QML 不支持 strokeGradient)
        ShapePath {
            id: baseArc
            strokeColor: root.baseColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: root.width / 2
                centerY: root.height / 2
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.startAngle
                sweepAngle: root.totalAngle
            }
        }

        // 2. 进度弧主体 (纯色, 与 QWidget 版本保持一致的设计语言)
        ShapePath {
            id: progressArc
            strokeColor: root.arcColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: root.width / 2
                centerY: root.height / 2
                radiusX: root.arcRadius
                radiusY: root.arcRadius
                startAngle: root.startAngle
                sweepAngle: root.progressAngle
            }
        }
    }

    // 数值文本 (居中, 与 QWidget 版本一致)
    Text {
        id: valueText
        anchors.centerIn: parent
        text: {
            if (root.showPercent) {
                let percent = ((root.value - root.minValue) / (root.maxValue - root.minValue)) * 100;
                return percent.toFixed(2) + "%";
            } else {
                return root.value.toFixed(2);
            }
        }
        color: root.textColor
        font.pixelSize: Math.min(root.width, root.height) / 10.0
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // 标题文本 (位于数值上方, 与 QWidget 版本一致: -arcRadius/2.5)
    Text {
        id: titleText
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: -root.arcRadius / 2.5
        }
        text: root.title
        color: root.titleColor
        font.pixelSize: Math.min(root.width, root.height) / 15.0
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // === 公共方法 ===
    function setValue(newValue: real) {
        const clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            privateData.previousValue = root.value;
            privateData.animationRunning = false;
            privateData.updatingFromExternal = true;
            root.value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function setValueAnimated(newValue: real) {
        const clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            privateData.previousValue = root.value;
            privateData.animationRunning = true;
            privateData.updatingFromExternal = true;
            root.value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function increaseValue(increment: real) {
        const actualIncrement = increment || 1.0;
        if (actualIncrement <= 0)
            return;

        const newValue = Math.min(root.maxValue, root.value + actualIncrement);
        if (newValue !== root.value) {
            root.setValueAnimated(newValue);
        }
    }

    function decreaseValue(decrement: real) {
        const actualDecrement = decrement || 1.0;
        if (actualDecrement <= 0)
            return;

        const newValue = Math.max(root.minValue, root.value - actualDecrement);
        if (newValue !== root.value) {
            root.setValueAnimated(newValue);
        }
    }

    function reset() {
        if (root.value !== root.minValue) {
            root.setValueAnimated(root.minValue);
            root.valueReset();
        }
    }

    // === 组件完成时初始化 ===
    Component.onCompleted: {
        // 初始化动画值
        privateData.previousValue = root.value;
    }
}
