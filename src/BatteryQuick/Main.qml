import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow

    width: 600
    height: 450
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

        // === 顶部：电池显示区域 ===
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 120

            Battery {
                id: battery
                anchors.centerIn: parent
                width: 200
                height: 80
            }
        }

        // === 控制面板区域 ===
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 3
            rowSpacing: 8
            columnSpacing: 10

            // 第一行：数值控制
            Label {
                text: qsTr("Current value:")
                Layout.alignment: Qt.AlignRight
            }

            Slider {
                id: valueSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                value: battery.value
                onMoved: {
                    if (pressed) {
                        battery.setValue(value);
                    }
                }
            }

            SpinBox {
                id: valueSpinBox
                from: 0
                to: 100
                value: battery.value
                editable: true
                onValueModified: battery.setValue(value)
            }

            // 第二行：报警阈值
            Label {
                text: qsTr("Alarm threshold:")
                Layout.alignment: Qt.AlignRight
            }

            Slider {
                id: alarmSlider
                Layout.fillWidth: true
                from: 0
                to: 50
                value: 20
                onMoved: {
                    if (pressed) {
                        battery.alarmValue = value;
                    }
                }
            }

            SpinBox {
                id: alarmSpinBox
                from: 0
                to: 50
                value: 20
                editable: true
                onValueModified: battery.alarmValue = value
            }

            // 第三行：电源颜色
            Label {
                text: qsTr("Power color:")
                Layout.alignment: Qt.AlignRight
            }

            Button {
                id: powerColorButton
                Layout.fillWidth: true
                text: battery.powerColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = battery.powerColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.powerColor = colorDialog.selectedColor;
                            powerColorButton.palette.buttonText = getContrastTextColor(battery.powerColor);
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
            }

            Item {
                Layout.fillWidth: true
            }

            // 第四行：报警颜色
            Label {
                text: qsTr("Alarm color:")
                Layout.alignment: Qt.AlignRight
            }

            Button {
                id: alarmColorButton
                Layout.fillWidth: true
                text: battery.alarmColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = battery.alarmColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.alarmColor = colorDialog.selectedColor;
                            alarmColorButton.palette.buttonText = getContrastTextColor(battery.alarmColor);
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
            }

            Item {
                Layout.fillWidth: true
            }

            // 第五行：边框颜色
            Label {
                text: qsTr("Border color:")
                Layout.alignment: Qt.AlignRight
            }

            Button {
                id: borderColorButton
                Layout.fillWidth: true
                text: battery.borderColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = battery.borderColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            battery.borderColor = colorDialog.selectedColor;
                            borderColorButton.palette.buttonText = getContrastTextColor(battery.borderColor);
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
            }

            Item {
                Layout.fillWidth: true
            }

            // 第六行：动画时长
            Label {
                id: durationLabel
                text: qsTr("Animation duration: 500ms")
                Layout.columnSpan: 1
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

            Item {
                Layout.fillWidth: true
            }

            // 第七行：操作按钮第一组
            Label {
                text: qsTr("Actions:")
                Layout.alignment: Qt.AlignRight
            }

            Button {
                text: qsTr("+10")
                Layout.fillWidth: true
                onClicked: battery.increaseValue(10)
            }

            Button {
                text: qsTr("Animate to 50")
                Layout.fillWidth: true
                onClicked: battery.setValueAnimated(50)
            }

            // 第八行：操作按钮第二组
            Item {} // 占位

            Button {
                text: qsTr("-10")
                Layout.fillWidth: true
                onClicked: battery.decreaseValue(10)
            }

            Button {
                text: qsTr("Reset")
                Layout.fillWidth: true
                onClicked: battery.reset()
            }

            // 第九行：充电控制
            Label {
                text: qsTr("Charging:")
                Layout.alignment: Qt.AlignRight
            }

            CheckBox {
                id: chargingCheckbox
                checked: battery.charging
                onToggled: battery.charging = checked
                Layout.columnSpan: 2
            }
        }

        // === 状态显示 ===
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Normal")
            horizontalAlignment: Text.AlignHCenter
            padding: 10
        }
    }

    // === 信号连接 ===
    Connections {
        target: battery

        function onAlarmStateChanged(isAlarm) {
            if (isAlarm) {
                statusLabel.text = qsTr("Status: Low battery alarm!");
                statusLabel.color = "red";
                statusLabel.font.bold = true;
            } else {
                statusLabel.text = qsTr("Status: Normal");
                statusLabel.color = "";
                statusLabel.font.bold = false;
            }
        }

        function onChargingChanged() {
            chargingCheckbox.checked = battery.charging;

            if (battery.charging) {
                statusLabel.text = qsTr("Status: Charging...");
                statusLabel.color = "blue";
            }
        }

        function onAnimationStarted(oldValue, newValue) {
            statusLabel.text = qsTr("Animating: %1% → %2%").arg(oldValue).arg(newValue);
            statusLabel.color = "orange";
        }

        function onAnimationFinished(value) {
            statusLabel.text = qsTr("Animation finished: %1%").arg(value);
            statusLabel.color = "green";
        }

        function onValueIncreased(newValue) {
            console.log("Value increased to:", newValue);
        }

        function onValueDecreased(newValue) {
            console.log("Value decreased to:", newValue);
        }

        function onValueReset() {
            console.log("Value reset");
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        battery.value = 75;
        alarmSlider.value = 20;
        alarmSpinBox.value = 20;
        animationDurationSlider.value = 500;
        powerColorButton.palette.buttonText = getContrastTextColor(battery.powerColor);
        borderColorButton.palette.buttonText = getContrastTextColor(battery.borderColor);
        alarmColorButton.palette.buttonText = getContrastTextColor(battery.alarmColor);
    }

    // 计算字体颜色的函数（WCAG标准）
    function getContrastTextColor(bgColor) {
        function normalize(x) {
            return x <= 0.03928 ? x / 12.92 : Math.pow((x + 0.055) / 1.055, 2.4);
        }

        function getRelativeLuminance(r, g, b) {
            return 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0) + 0.0722 * normalize(b / 255.0);
        }

        var luminance = getRelativeLuminance(bgColor.r * 255, bgColor.g * 255, bgColor.b * 255);
        return luminance > 0.179 ? "black" : "white";
    }
}
