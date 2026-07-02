import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 620
    height: 280
    visible: true
    title: qsTr("Password Input Quick Example")

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        // === 左侧：密码输入演示区域 ===
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            GroupBox {
                title: qsTr("Basic Password Field")
                Layout.fillWidth: true

                PasswordInput {
                    id: basicPasswordInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    placeholderText: qsTr("Enter your password here")
                }
            }

            GroupBox {
                title: qsTr("Password Field with Custom Placeholder")
                Layout.fillWidth: true

                PasswordInput {
                    id: customPasswordInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    placeholderText: qsTr("Minimum 8 characters with special symbols")
                }
            }

            // 密码信息显示
            Label {
                id: infoLabel
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Basic: %1 | Length: %2\nCustom: %3 | Length: %4").arg(basicPasswordInput.passwordVisible ? qsTr("Visible") : qsTr("Hidden")).arg(basicPasswordInput.text.length).arg(customPasswordInput.passwordVisible ? qsTr("Visible") : qsTr("Hidden")).arg(customPasswordInput.text.length)
                font.pixelSize: 12
            }

            Item {
                Layout.fillHeight: true
            }
        }

        // === 右侧：控制面板 ===
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // 显示设置
            GroupBox {
                title: qsTr("Display Settings")
                Layout.fillWidth: true

                CheckBox {
                    id: capsLockCheckbox
                    text: qsTr("Enable CapsLock Warning")
                    checked: true
                    onToggled: {
                        basicPasswordInput.capsLockWarningEnabled = checked;
                        customPasswordInput.capsLockWarningEnabled = checked;
                    }
                }
            }

            // 操作按钮
            GroupBox {
                title: qsTr("Actions")
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent

                    Button {
                        text: qsTr("Show All")
                        Layout.fillWidth: true
                        onClicked: {
                            basicPasswordInput.setPasswordVisible(true);
                            customPasswordInput.setPasswordVisible(true);
                        }
                    }

                    Button {
                        text: qsTr("Hide All")
                        Layout.fillWidth: true
                        onClicked: {
                            basicPasswordInput.setPasswordVisible(false);
                            customPasswordInput.setPasswordVisible(false);
                        }
                    }

                    Button {
                        text: qsTr("Clear All")
                        Layout.fillWidth: true
                        onClicked: {
                            basicPasswordInput.clear();
                            customPasswordInput.clear();
                        }
                    }

                    Button {
                        text: qsTr("Validate")
                        Layout.fillWidth: true
                        onClicked: mainWindow.validatePasswords()
                    }

                    Button {
                        text: qsTr("Reset")
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        onClicked: mainWindow.resetAll()
                    }
                }
            }

            // 状态显示
            Label {
                id: statusLabel
                Layout.fillWidth: truep
                text: qsTr("Ready")
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
                padding: 4
                background: Rectangle {
                    anchors.fill: parent
                    color: "#f2f2f7"
                    border.color: "#d1d1d6"
                    border.width: 1
                    radius: 4
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    // === 工具函数 ===

    function validatePasswords(): void {
        const inputs = [basicPasswordInput, customPasswordInput];
        let allValid = true;
        const messages = [];

        for (const input of inputs) {
            if (input.text.length > 0 && input.text.length < 6) {
                messages.push(qsTr("'%1' is too short (min 6 characters)").arg(input.placeholderText.substring(0, 20)));
                allValid = false;
            }
        }

        if (allValid && messages.length === 0) {
            statusLabel.text = "✓ " + qsTr("All passwords are valid");
        } else if (messages.length === 0) {
            statusLabel.text = "ℹ " + qsTr("No passwords to validate");
        } else {
            statusLabel.text = "✗ " + messages.join("; ");
        }
    }

    function resetAll(): void {
        basicPasswordInput.clear();
        customPasswordInput.clear();
        basicPasswordInput.setPasswordVisible(false);
        customPasswordInput.setPasswordVisible(false);
        basicPasswordInput.capsLockWarningEnabled = true;
        customPasswordInput.capsLockWarningEnabled = true;
        capsLockCheckbox.checked = true;
        statusLabel.text = qsTr("Reset complete");
    }
}
