import QtQuick

Item {
    id: root

    // === 属性定义 ===
    property int value: 75
    property int alarmValue: 20
    property color borderColor: "#505050"
    property color powerColor: "#41cd52"
    property color alarmColor: "#fa7671"
    property int animationDuration: 500
    property bool charging: false

    // === 信号定义 ===
    signal valueIncreased(int newValue)
    signal valueDecreased(int newValue)
    signal valueReset
    signal animationStarted(int oldValue, int newValue)
    signal animationFinished(int value)
    signal animationStopped(int currentValue)
    signal alarmStateChanged(bool isInAlarm)

    // === 计算属性 ===
    readonly property bool isInAlarmState: value <= alarmValue
    readonly property bool isAnimating: privateData.animationRunning
    readonly property color currentPowerColor: isInAlarmState ? alarmColor : powerColor

    // === 尺寸建议 ===
    implicitWidth: 150
    implicitHeight: 80

    // === 私有数据 ===
    QtObject {
        id: privateData
        property bool wasInAlarmState: false
        property bool animationRunning: false
        property int previousValue: root.value
        property bool updatingFromExternal: false
    }

    // === 动画系统 ===
    Behavior on value {
        id: valueAnimation
        enabled: root.animationDuration > 0 && privateData.animationRunning
        NumberAnimation {
            duration: root.animationDuration
            easing.type: Easing.InOutQuad

            onStarted: {
                privateData.animationRunning = true;
                root.animationStarted(privateData.previousValue, root.value);
            }

            onStopped: {
                privateData.animationRunning = false;
                root.animationFinished(root.value);
                root.checkAlarmState();
            }
        }
    }

    // === 状态监控 ===
    onValueChanged: {
        if (!privateData.updatingFromExternal) {
            root.checkAlarmState();
        }
        privateData.previousValue = value;
    }

    onAlarmValueChanged: checkAlarmState()

    // === 电池外观组件 ===

    // 电池主体
    Rectangle {
        id: rootBody
        width: parent.width - rootHead.width
        height: parent.height
        radius: Math.max(2, height * 0.05)
        color: "transparent"
        border {
            color: root.borderColor
            width: Math.max(1, height * 0.03)
        }

        // 电量填充
        Rectangle {
            id: powerLevel
            width: (rootBody.width - rootBody.border.width * 2) * Math.max(0, Math.min(100, root.value)) / 100
            height: rootBody.height - rootBody.border.width * 2
            x: rootBody.border.width
            y: rootBody.border.width
            radius: Math.max(0, rootBody.radius - rootBody.border.width)
            color: root.currentPowerColor
        }

        // 数值显示
        Text {
            id: valueText
            text: root.value + "%"
            font {
                pixelSize: Math.max(12, rootBody.height * 0.3)
                bold: true
            }
            color: {
                if (root.charging)
                    return "#1e1e1e";
                return root.isInAlarmState ? root.alarmColor : "#404142";
            }
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            width: root.charging ? parent.width * 0.65 : parent.width
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // 充电符号
        Text {
            id: chargingSymbol
            visible: root.charging
            text: "⚡"
            font.pixelSize: Math.max(12, rootBody.height * 0.25)
            color: root.isInAlarmState ? root.alarmColor : "#1e1e1e"
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: valueText.right
                right: parent.right
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // 电池头部
    Rectangle {
        id: rootHead
        width: Math.max(5, parent.width * 0.07)
        height: Math.max(8, rootBody.height * 0.35)
        x: rootBody.width - 1
        y: (parent.height - height) / 2
        color: root.borderColor
    }

    // === 公共方法 ===
    function setValue(newValue) {
        var clampedValue = Math.max(0, Math.min(100, newValue));
        if (clampedValue !== value) {
            privateData.previousValue = value;
            privateData.animationRunning = false;
            privateData.updatingFromExternal = true;
            value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function setValueAnimated(newValue) {
        var clampedValue = Math.max(0, Math.min(100, newValue));
        if (clampedValue !== value) {
            privateData.previousValue = value;
            privateData.animationRunning = true;
            privateData.updatingFromExternal = true;
            value = clampedValue;
            privateData.updatingFromExternal = false;
        }
    }

    function increaseValue(increment) {
        if (increment <= 0)
            return;
        var actualIncrement = increment || 1;
        var newValue = Math.min(100, value + actualIncrement);

        if (newValue !== value) {
            privateData.previousValue = value;
            setValue(newValue);
            valueIncreased(newValue);
        }
    }

    function decreaseValue(decrement) {
        if (decrement <= 0)
            return;
        var actualDecrement = decrement || 1;
        var newValue = Math.max(0, value - actualDecrement);

        if (newValue !== value) {
            privateData.previousValue = value;
            setValue(newValue);
            valueDecreased(newValue);
        }
    }

    function reset() {
        if (value !== 0) {
            privateData.previousValue = value;
            setValue(0);
            valueReset();
        }
    }

    function startCharging() {
        charging = true;
    }

    function stopCharging() {
        charging = false;
    }

    function toggleCharging() {
        charging = !charging;
    }

    // === 私有方法 ===
    function checkAlarmState() {
        var currentAlarmState = (value <= alarmValue);
        if (currentAlarmState !== privateData.wasInAlarmState) {
            alarmStateChanged(currentAlarmState);
            privateData.wasInAlarmState = currentAlarmState;
        }
    }
}
