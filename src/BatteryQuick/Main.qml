import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: root

    width: 650
    height: 400
    visible: true
    title: qsTr("Battery Widget Example")

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 电池和滑动条布局
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Battery {
                id: battery
                Layout.fillWidth: true
                Layout.fillHeight: true
                value: 75
                alarmValue: 20
                animationDuration: 500
                charging: false
            }

            Slider {
                id: slider
                Layout.fillWidth: true
                Layout.fillHeight: true
                orientation: Qt.Vertical
                from: 0
                to: 100
                value: 75

                onValueChanged: {
                    if (pressed) {
                        if (animationCheckbox.checked) {
                            battery.setValueAnimated(value);
                        } else {
                            battery.value = value;
                        }
                    }
                }
            }
        }

        // 充电和动画控制布局
        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                id: chargingCheckbox
                text: qsTr("Charging state")
                checked: false
                onToggled: {
                    if (checked !== undefined) {
                        battery.charging = checked;
                    }
                }
            }

            CheckBox {
                id: animationCheckbox
                text: qsTr("Enable animation")
                checked: true
            }
        }

        // 颜色选择布局
        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Battery level color")
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.powerColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor !== undefined) {
                            battery.powerColor = colorDialog.selectedColor;
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }
            }

            Button {
                text: qsTr("Alarm color")
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.alarmColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor !== undefined) {
                            battery.alarmColor = colorDialog.selectedColor;
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }
            }

            Button {
                text: qsTr("Border color")
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.borderColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor !== undefined) {
                            battery.borderColor = colorDialog.selectedColor;
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }
            }
        }

        // 阈值和动画时长布局
        GridLayout {
            Layout.fillWidth: true
            columns: 2

            Label {
                id: alarmLabel
                text: qsTr("Alarm threshold: 20%")
                Layout.fillWidth: true
            }

            Slider {
                id: alarmSlider
                Layout.fillWidth: true
                from: 0
                to: 50
                value: 20
                onValueChanged: {
                    battery.alarmValue = value;
                    alarmLabel.text = qsTr("Alarm threshold: %1%").arg(value);
                }
            }

            Label {
                id: durationLabel
                text: qsTr("Animation duration: 500ms")
                Layout.fillWidth: true
            }

            Slider {
                id: animationDurationSlider
                Layout.fillWidth: true
                from: 100
                to: 2000
                value: 500
                onValueChanged: {
                    battery.animationDuration = value;
                    durationLabel.text = qsTr("Animation duration: %1ms").arg(value);
                }
            }
        }

        // 快速操作布局
        RowLayout {
            Layout.fillWidth: true

            Button {
                text: "+10%"
                Layout.fillWidth: true
                onClicked: battery.increaseValue(10)
            }

            Button {
                text: "-10%"
                Layout.fillWidth: true
                onClicked: battery.decreaseValue(10)
            }

            Button {
                text: qsTr("Reset")
                Layout.fillWidth: true
                onClicked: battery.reset()
            }
        }

        // 状态显示
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Normal")
            horizontalAlignment: Text.AlignHCenter
        }
    }

    // 连接电池信号
    Connections {
        target: battery

        // 报警状态变化
        function onAlarmStateChanged(isAlarm) {
            if (isAlarm === true) {
                statusLabel.text = qsTr("Status: Low battery alarm!");
                statusLabel.color = "red";
                statusLabel.font.bold = true;
            } else {
                statusLabel.text = qsTr("Status: Normal");
                statusLabel.color = "";
                statusLabel.font.bold = false;
            }
        }

        // 充电状态变化
        function onChargingChanged(charging) {
            if (charging !== undefined) {
                chargingCheckbox.checked = charging;
            }
            if (charging === true) {
                statusLabel.text = qsTr("Status: Charging...");
                statusLabel.color = "blue";
            } else {
                // 当停止充电时，不改变状态文本，保持当前状态
            }
        }

        // 动画开始
        function onAnimationStarted(oldValue, newValue) {
            statusLabel.text = qsTr("Animation: %1% → %2%").arg(oldValue).arg(newValue);
        }

        // 动画完成
        function onAnimationFinished(value) {
            statusLabel.text = qsTr("The animation is completed: %1%").arg(value);
        }

        // 值增加
        function onValueIncreased(newValue) {
            console.log("Value increased to:", newValue);
        }

        // 值减少
        function onValueDecreased(newValue) {
            console.log("Value decreased to:", newValue);
        }

        // 值重置
        function onValueReset() {
            console.log("Value reset");
        }
    }
}
