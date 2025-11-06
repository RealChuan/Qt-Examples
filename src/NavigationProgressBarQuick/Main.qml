import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow

    width: 800
    height: 380
    visible: true
    title: qsTr("Navigation Progress Bar Example")

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 进度条和步骤控制布局
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            NavigationProgressBar {
                id: progressBar
                Layout.fillWidth: true
                Layout.fillHeight: true
                messageList: ["Order Placed", "Payment Confirmed", "Processing", "Shipping", "Delivered"]
                step: 2
                spacing: 10

                onStepChanged: {
                    stepSlider.value = step;
                    stepLabel.text = qsTr("Current Step: %1").arg(step);
                    statusLabel.text = qsTr("Step changed to: %1").arg(step);
                }

                onProgressCompleted: {
                    statusLabel.text = qsTr("All steps completed!");
                }
            }

            Slider {
                id: stepSlider
                Layout.fillHeight: true
                orientation: Qt.Vertical
                from: 0
                to: progressBar.maxStep
                value: progressBar.step
                stepSize: 1
                onValueChanged: {
                    if (pressed && Math.abs(value - progressBar.step) > 0.1) {
                        progressBar.setStep(Math.round(value));
                    }
                }
            }

            // 步骤控制面板
            ColumnLayout {
                spacing: 5

                Label {
                    id: stepLabel
                    text: qsTr("Current Step: %1").arg(progressBar.step)
                    font.bold: true
                }

                Button {
                    text: qsTr("Previous Step")
                    onClicked: progressBar.previous()
                    enabled: progressBar.step > 0
                }

                Button {
                    text: qsTr("Next Step")
                    onClicked: progressBar.next()
                    enabled: progressBar.step < progressBar.maxStep
                }

                Button {
                    text: qsTr("Reset")
                    onClicked: progressBar.reset()
                }
            }
        }

        // 外观控制区域
        GroupBox {
            title: qsTr("Appearance Controls")
            Layout.fillWidth: true

            GridLayout {
                columns: 2
                width: parent.width

                // 背景颜色选择
                Label {
                    text: qsTr("Background Color:")
                    Layout.alignment: Qt.AlignRight
                }

                Button {
                    id: backgroundColorButton
                    Layout.fillWidth: true
                    text: progressBar.backgroundColor.toString().toUpperCase()
                    onClicked: {
                        colorDialog.selectedColor = progressBar.backgroundColor;
                        colorDialog.accepted.connect(function () {
                            if (colorDialog.selectedColor) {
                                progressBar.backgroundColor = colorDialog.selectedColor;
                                backgroundColorButton.text = progressBar.backgroundColor.toString().toUpperCase();
                                backgroundColorButton.palette.buttonText = getContrastTextColor(progressBar.backgroundColor);
                            }
                            colorDialog.accepted.disconnect(arguments.callee);
                        });
                        colorDialog.open();
                    }

                    background: Rectangle {
                        color: progressBar.backgroundColor
                        border.color: "gray"
                        border.width: 1
                    }
                }

                // 当前步骤颜色选择
                Label {
                    text: qsTr("Current Step Color:")
                    Layout.alignment: Qt.AlignRight
                }

                Button {
                    id: currentColorButton
                    Layout.fillWidth: true
                    text: progressBar.currentBackgroundColor.toString().toUpperCase()
                    onClicked: {
                        colorDialog.selectedColor = progressBar.currentBackgroundColor;
                        colorDialog.accepted.connect(function () {
                            if (colorDialog.selectedColor) {
                                progressBar.currentBackgroundColor = colorDialog.selectedColor;
                                currentColorButton.text = progressBar.currentBackgroundColor.toString().toUpperCase();
                                currentColorButton.palette.buttonText = getContrastTextColor(progressBar.currentBackgroundColor);
                            }
                            colorDialog.accepted.disconnect(arguments.callee);
                        });
                        colorDialog.open();
                    }

                    background: Rectangle {
                        color: progressBar.currentBackgroundColor
                        border.color: "gray"
                        border.width: 1
                    }
                }

                // 文字颜色选择
                Label {
                    text: qsTr("Text Color:")
                    Layout.alignment: Qt.AlignRight
                }

                Button {
                    id: foregroundColorButton
                    Layout.fillWidth: true
                    text: progressBar.foregroundColor.toString().toUpperCase()
                    onClicked: {
                        colorDialog.selectedColor = progressBar.foregroundColor;
                        colorDialog.accepted.connect(function () {
                            if (colorDialog.selectedColor) {
                                progressBar.foregroundColor = colorDialog.selectedColor;
                                foregroundColorButton.text = progressBar.foregroundColor.toString().toUpperCase();
                                foregroundColorButton.palette.buttonText = getContrastTextColor(progressBar.foregroundColor);
                            }
                            colorDialog.accepted.disconnect(arguments.callee);
                        });
                        colorDialog.open();
                    }

                    background: Rectangle {
                        color: progressBar.foregroundColor
                        border.color: "gray"
                        border.width: 1
                    }
                }

                // 间距控制
                Label {
                    id: spacingLabel
                    text: qsTr("Spacing: %1").arg(progressBar.spacing)
                    Layout.alignment: Qt.AlignRight
                }

                Slider {
                    id: spacingSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 50
                    value: progressBar.spacing
                    onValueChanged: {
                        progressBar.spacing = Math.round(value);
                        spacingLabel.text = qsTr("Spacing: %1").arg(Math.round(value));
                    }
                }
            }
        }

        // 步骤信息编辑区域
        GroupBox {
            title: qsTr("Step Management")
            Layout.fillWidth: true

            RowLayout {
                width: parent.width

                TextField {
                    id: messageEdit
                    Layout.fillWidth: true
                    placeholderText: qsTr("Enter step description...")

                    onAccepted: {
                        var text = messageEdit.text.trim();
                        if (text !== "") {
                            var messages = progressBar.messageList.concat([text]);
                            progressBar.messageList = messages;
                            messageEdit.text = "";
                            stepSlider.to = progressBar.maxStep;
                        }
                    }
                }

                Button {
                    text: qsTr("Add Step")
                    onClicked: {
                        var text = messageEdit.text.trim();
                        if (text !== "") {
                            var messages = progressBar.messageList.concat([text]);
                            progressBar.messageList = messages;
                            messageEdit.text = "";
                            stepSlider.to = progressBar.maxStep;
                        }
                    }
                }

                Button {
                    text: qsTr("Clear All")
                    onClicked: {
                        progressBar.messageList = [];
                        stepSlider.to = progressBar.maxStep;
                    }
                }
            }
        }

        // 状态显示
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Ready")
            horizontalAlignment: Text.AlignHCenter
            padding: 10
            background: Rectangle {
                border.color: "gray"
                border.width: 1
                color: "transparent"
            }
        }
    }

    // 初始化颜色按钮的文本颜色
    Component.onCompleted: {
        backgroundColorButton.palette.buttonText = getContrastTextColor(progressBar.backgroundColor);
        currentColorButton.palette.buttonText = getContrastTextColor(progressBar.currentBackgroundColor);
        foregroundColorButton.palette.buttonText = getContrastTextColor(progressBar.foregroundColor);
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
