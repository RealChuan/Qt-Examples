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

    // 颜色属性
    property color arcColor: "#4da1ff"
    property color textColor: "#4da1ff"
    property color titleColor: "#505050"
    property color baseColor: "#b3b3b3"
    property color backgroundColor: "transparent"

    // 动画属性
    property int animationDuration: 500

    // 尺寸属性
    property real arcWidth: Math.min(width, height) * 0.1

    // === 计算属性 ===
    readonly property bool isAnimating: privateData.animationRunning
    readonly property real progress: (value - minValue) / (maxValue - minValue)
    readonly property real totalAngle: endAngle - startAngle
    readonly property real progressAngle: totalAngle * progress

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
            // 直接设置值，无动画
            privateData.previousValue = value;

            // 检查数值增减
            if (value > privateData.previousValue) {
                valueIncreased(value);
            } else if (value < privateData.previousValue) {
                valueDecreased(value);
            }
        }
    }

    // === 外观组件 ===

    // 背景
    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: root.backgroundColor
        radius: Math.min(width, height) * 0.5
    }

    // 使用 Shape 绘制圆弧
    Shape {
        id: progressShape
        anchors.fill: parent
        vendorExtensionsEnabled: true
        preferredRendererType: Shape.CurveRenderer

        // 背景圆弧
        ShapePath {
            id: baseArc
            strokeColor: root.baseColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: root.width / 2
                centerY: root.height / 2
                radiusX: Math.min(root.width, root.height) / 2 - root.arcWidth / 2
                radiusY: Math.min(root.width, root.height) / 2 - root.arcWidth / 2
                startAngle: root.startAngle
                sweepAngle: root.totalAngle
            }
        }

        // 进度圆弧
        ShapePath {
            id: progressArc
            strokeColor: root.arcColor
            fillColor: "transparent"
            strokeWidth: root.arcWidth
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: root.width / 2
                centerY: root.height / 2
                radiusX: Math.min(root.width, root.height) / 2 - root.arcWidth / 2
                radiusY: Math.min(root.width, root.height) / 2 - root.arcWidth / 2
                startAngle: root.startAngle
                sweepAngle: root.progressAngle
            }
        }
    }

    // 数值文本
    Text {
        id: valueText
        anchors.centerIn: parent
        anchors.verticalCenterOffset: parent.height * 0.1
        text: {
            if (root.showPercent) {
                var percent = ((root.value - root.minValue) / (root.maxValue - root.minValue)) * 100;
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

    // 标题文本
    Text {
        id: titleText
        anchors {
            top: valueText.bottom
            horizontalCenter: parent.horizontalCenter
            topMargin: Math.min(root.width, root.height) * 0.05
        }
        text: root.title
        color: root.titleColor
        font.pixelSize: Math.min(root.width, root.height) / 15.0
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // === 公共方法 ===
    function setValue(newValue) {
        var clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            privateData.previousValue = root.value;
            privateData.animationRunning = false;
            privateData.updatingFromExternal = true;
            root.value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function setValueAnimated(newValue) {
        var clampedValue = Math.max(root.minValue, Math.min(root.maxValue, newValue));
        if (clampedValue !== root.value) {
            privateData.previousValue = root.value;
            privateData.animationRunning = true;
            privateData.updatingFromExternal = true;
            root.value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function increaseValue(increment) {
        var actualIncrement = increment || 1.0;
        if (actualIncrement <= 0)
            return;

        var newValue = Math.min(root.maxValue, root.value + actualIncrement);
        if (newValue !== root.value) {
            root.setValueAnimated(newValue);
        }
    }

    function decreaseValue(decrement) {
        var actualDecrement = decrement || 1.0;
        if (actualDecrement <= 0)
            return;

        var newValue = Math.max(root.minValue, root.value - actualDecrement);
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
