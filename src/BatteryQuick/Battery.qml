import QtQuick

Item {
    id: battery

    // 属性定义
    property int value: 0
    property int alarmValue: 20
    property int animationDuration: 500
    property color borderColor: "#505050"
    property color powerColor: "#41cd52"
    property color alarmColor: "#fa7671"
    property bool charging: false

    // 信号定义
    signal valueIncreased(int newValue)
    signal valueDecreased(int newValue)
    signal valueReset
    signal animationStarted(int oldValue, int newValue)
    signal animationFinished(int value)
    signal animationStopped(int currentValue)
    signal alarmStateChanged(bool isInAlarm)

    // 计算属性
    readonly property bool isInAlarmState: value <= alarmValue
    readonly property bool isAnimating: privateData.animationRunning

    // 尺寸建议
    implicitWidth: 150
    implicitHeight: 80

    // 动画
    Behavior on value {
        id: valueAnimation
        enabled: battery.animationDuration > 0  // 确保动画有效
        NumberAnimation {
            id: numberAnimation
            duration: battery.animationDuration
            easing.type: Easing.InOutQuad

            onStarted: {
                privateData.animationRunning = true;
            }

            onStopped: {
                privateData.animationRunning = false;
                battery.animationFinished(battery.value);
            }

            onRunningChanged: {
                if (!running) {
                    privateData.animationRunning = false;
                }
            }
        }
    }

    // 报警状态检查
    onValueChanged: {
        checkAlarmState();
    }

    onAlarmValueChanged: checkAlarmState()

    function checkAlarmState() {
        var currentAlarmState = (value <= alarmValue);
        if (currentAlarmState !== privateData.wasInAlarmState) {
            alarmStateChanged(currentAlarmState);
            privateData.wasInAlarmState = currentAlarmState;
        }
    }

    // 私有数据
    QtObject {
        id: privateData
        property bool wasInAlarmState: false
        property bool animationRunning: false
    }

    // 电池主体
    Rectangle {
        id: batteryBody
        width: parent.width - batteryHead.width
        height: parent.height
        radius: height * 0.05
        color: "transparent"
        border.color: battery.borderColor
        border.width: 2

        // 电量填充
        Rectangle {
            id: powerLevel
            width: (batteryBody.width - 4) * Math.max(0, Math.min(100, battery.value)) / 100
            height: batteryBody.height - 4
            x: 2
            y: 2
            radius: Math.max(0, batteryBody.radius - 1)
            color: battery.isInAlarmState ? battery.alarmColor : battery.powerColor
        }

        // 充电状态下：数值显示在左侧2/3区域
        Text {
            id: valueText
            text: battery.value + "%"
            font.pixelSize: batteryBody.height * 0.5
            font.bold: true
            color: battery.charging ? "#1e1e1e" : (battery.isInAlarmState ? battery.alarmColor : "#404142")
            anchors {
                top: batteryBody.top
                bottom: batteryBody.bottom
                left: batteryBody.left
            }
            width: battery.charging ? batteryBody.width * 2.0 / 3.0 : batteryBody.width
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // 充电状态下：闪电显示在右侧1/3区域
        Text {
            id: chargingSymbol
            visible: battery.charging === true
            text: "⚡"
            font.pixelSize: batteryBody.height * 0.4
            color: "white"
            anchors {
                top: batteryBody.top
                bottom: batteryBody.bottom
                left: valueText.right
                right: batteryBody.right
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // 电池头部
    Rectangle {
        id: batteryHead
        width: parent.width * 0.09
        height: batteryBody.height * 0.4
        x: batteryBody.width - 1
        y: (parent.height - height) / 2
        radius: height / 30.0
        color: battery.borderColor
    }

    // 公共方法
    function setValue(newValue) {
        newValue = Math.max(0, Math.min(100, newValue));
        if (newValue !== value) {
            value = newValue;
        }
    }

    function setValueAnimated(newValue) {
        newValue = Math.max(0, Math.min(100, newValue));
        if (newValue !== value) {
            animationStarted(value, newValue);
            value = newValue;
        }
    }

    function increaseValue(increment) {
        if (increment <= 0)
            return;
        var newValue = Math.min(100, value + (increment || 1));
        if (newValue !== value) {
            setValue(newValue);
            valueIncreased(newValue);
        }
    }

    function decreaseValue(decrement) {
        if (decrement <= 0)
            return;
        var newValue = Math.max(0, value - (decrement || 1));
        if (newValue !== value) {
            setValue(newValue);
            valueDecreased(newValue);
        }
    }

    function reset() {
        if (value !== 0) {
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
}
