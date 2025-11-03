import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: root

    width: 320
    height: 370
    visible: true
    title: qsTr("Battery Quick Example")

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

        // 使用GridLayout来组织控件，与C++版本一致
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            // 第一行：充电状态和动画控制
            CheckBox {
                id: chargingCheckbox
                text: qsTr("Charging state")
                checked: false
                Layout.columnSpan: 1
                onToggled: {
                    battery.charging = checked;
                }
            }

            CheckBox {
                id: animationCheckbox
                text: qsTr("Enable animation")
                checked: true
                Layout.columnSpan: 1
            }

            // 第二行：电池颜色选择
            Label {
                text: qsTr("Battery color:")
            }

            Button {
                id: powerColorButton
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.powerColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.powerColor = colorDialog.selectedColor;
                            updateColorButton(powerColorButton, colorDialog.selectedColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: battery.powerColor
                    border.color: "gray"
                    border.width: 1
                }

                contentItem: Item {} // 隐藏默认文本
            }

            // 第三行：报警颜色选择
            Label {
                text: qsTr("Alarm color:")
            }

            Button {
                id: alarmColorButton
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.alarmColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.alarmColor = colorDialog.selectedColor;
                            updateColorButton(alarmColorButton, colorDialog.selectedColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: battery.alarmColor
                    border.color: "gray"
                    border.width: 1
                }

                contentItem: Item {} // 隐藏默认文本
            }

            // 第四行：边框颜色选择
            Label {
                text: qsTr("Border color:")
            }

            Button {
                id: borderColorButton
                Layout.fillWidth: true
                onClicked: {
                    colorDialog.selectedColor = battery.borderColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.borderColor = colorDialog.selectedColor;
                            updateColorButton(borderColorButton, colorDialog.selectedColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: battery.borderColor
                    border.color: "gray"
                    border.width: 1
                }

                contentItem: Item {} // 隐藏默认文本
            }

            // 第五行：报警阈值
            Label {
                id: alarmLabel
                text: qsTr("Alarm threshold: 20%")
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

            // 第六行：动画时长
            Label {
                id: durationLabel
                text: qsTr("Animation duration: 500ms")
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
            padding: 10
        }
    }

    // 更新颜色按钮预览的函数
    function updateColorButton(button, color) {
        // 颜色已经在background中自动更新，因为绑定了电池的颜色属性
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
        function onChargingChanged() {
            if (battery.charging === true) {
                statusLabel.text = qsTr("Status: Charging...");
                statusLabel.color = "blue";
            } else {
                // 当停止充电时，不改变状态文本，保持当前状态
            }
        }

        // 动画开始
        function onAnimationStarted(oldValue, newValue) {
            statusLabel.text = qsTr("Animation: %1% → %2%").arg(oldValue).arg(newValue);
            statusLabel.color = "green";
        }

        // 动画完成
        function onAnimationFinished(value) {
            statusLabel.text = qsTr("The animation is completed: %1%").arg(value);
            statusLabel.color = "";
        }
    }

    // 初始化
    Component.onCompleted: {
        // 设置初始值
        battery.value = 75;
        // 初始化颜色按钮预览（通过绑定自动处理）
    }
}
