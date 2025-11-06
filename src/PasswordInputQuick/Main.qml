import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 600
    height: 500
    visible: true
    title: qsTr("Password Input Quick Example")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 基本密码输入框
        GroupBox {
            title: qsTr("Basic Password Input")
            Layout.fillWidth: true

            ColumnLayout {
                width: parent.width

                PasswordInput {
                    id: basicPasswordInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    placeholderText: qsTr("Enter your password here")

                    onTextChanged: {
                        mainWindow.updateStatus(qsTr("Basic password input updated"));
                    }
                }
            }
        }

        // 带自定义提示的密码框
        GroupBox {
            title: qsTr("Password Input with Custom Placeholder")
            Layout.fillWidth: true

            ColumnLayout {
                width: parent.width

                PasswordInput {
                    id: customPasswordInput
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    placeholderText: qsTr("Minimum 8 characters with special symbols")

                    onTextChanged: {
                        mainWindow.updateStatus(qsTr("Custom password input updated"));
                    }
                }
            }
        }

        // 控制面板
        GroupBox {
            title: qsTr("Controls")
            Layout.fillWidth: true

            GridLayout {
                columns: 3
                width: parent.width

                // 显示所有密码按钮
                Button {
                    text: qsTr("Show All Passwords")
                    Layout.fillWidth: true
                    onClicked: {
                        mainWindow.showAllPasswords();
                        mainWindow.updateStatus(qsTr("All passwords are now visible"));
                    }
                }

                // 隐藏所有密码按钮
                Button {
                    text: qsTr("Hide All Passwords")
                    Layout.fillWidth: true
                    onClicked: {
                        mainWindow.hideAllPasswords();
                        mainWindow.updateStatus(qsTr("All passwords are now hidden"));
                    }
                }

                // 清除所有密码按钮
                Button {
                    text: qsTr("Clear All Passwords")
                    Layout.fillWidth: true
                    onClicked: {
                        mainWindow.clearAllPasswords();
                        mainWindow.updateStatus(qsTr("All passwords cleared"));
                    }
                }

                // 验证按钮
                Button {
                    text: qsTr("Validate Passwords")
                    Layout.columnSpan: 3
                    Layout.fillWidth: true
                    onClicked: {
                        mainWindow.validatePasswords();
                    }
                }
            }
        }

        // 自定义图标设置
        GroupBox {
            title: qsTr("Custom Icon Settings")
            Layout.fillWidth: true

            RowLayout {
                width: parent.width

                TextField {
                    id: visibleIconInput
                    Layout.fillWidth: true
                    placeholderText: qsTr("Visible icon (text or emoji)")
                    text: "👁️"
                }

                TextField {
                    id: hiddenIconInput
                    Layout.fillWidth: true
                    placeholderText: qsTr("Hidden icon (text or emoji)")
                    text: "🔒"
                }

                Button {
                    text: qsTr("Apply Icons")
                    onClicked: {
                        mainWindow.applyCustomIcons();
                        mainWindow.updateStatus(qsTr("Custom icons applied"));
                    }
                }
            }
        }

        // 状态显示
        GroupBox {
            title: qsTr("Status")
            Layout.fillWidth: true

            Rectangle {
                width: parent.width
                radius: 4

                Text {
                    id: statusLabel
                    anchors.centerIn: parent
                    text: qsTr("Ready")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // 密码信息显示
        GroupBox {
            title: qsTr("Password Information")
            Layout.fillWidth: true

            ColumnLayout {
                width: parent.width
                spacing: 5

                Text {
                    text: qsTr("Basic Input: ") + (basicPasswordInput.passwordVisible ? qsTr("Visible") : qsTr("Hidden")) + " | " + qsTr("Length: ") + basicPasswordInput.text.length
                    font.pixelSize: 12
                }

                Text {
                    text: qsTr("Custom Input: ") + (customPasswordInput.passwordVisible ? qsTr("Visible") : qsTr("Hidden")) + " | " + qsTr("Length: ") + customPasswordInput.text.length
                    font.pixelSize: 12
                }
            }
        }
    }

    // 收集所有密码输入框
    function getAllPasswordInputs() {
        return [basicPasswordInput, customPasswordInput];
    }

    // 显示所有密码
    function showAllPasswords() {
        var inputs = getAllPasswordInputs();
        for (var i = 0; i < inputs.length; i++) {
            if (!inputs[i].passwordVisible) {
                inputs[i].togglePasswordVisibility();
            }
        }
    }

    // 隐藏所有密码
    function hideAllPasswords() {
        var inputs = getAllPasswordInputs();
        for (var i = 0; i < inputs.length; i++) {
            if (inputs[i].passwordVisible) {
                inputs[i].togglePasswordVisibility();
            }
        }
    }

    // 清除所有密码
    function clearAllPasswords() {
        var inputs = getAllPasswordInputs();
        for (var i = 0; i < inputs.length; i++) {
            inputs[i].clear();
        }
    }

    // 验证密码
    function validatePasswords() {
        var inputs = getAllPasswordInputs();
        var allValid = true;
        var messages = [];

        for (var i = 0; i < inputs.length; i++) {
            var input = inputs[i];
            if (input.text.length > 0) {
                if (input.text.length < 6) {
                    var inputName = input === basicPasswordInput ? qsTr("Basic input") : qsTr("Custom input");
                    messages.push(qsTr("%1 is too short (minimum 6 characters)").arg(inputName));
                    allValid = false;
                }
            }
        }

        if (allValid && messages.length === 0) {
            updateStatus("✓ " + qsTr("All passwords are valid"));
        } else if (messages.length === 0) {
            updateStatus("ℹ " + qsTr("No passwords to validate"));
        } else {
            updateStatus("✗ " + messages.join("; "));
        }
    }

    // 应用自定义图标
    function applyCustomIcons() {
        var inputs = getAllPasswordInputs();
        var visibleIcon = visibleIconInput.text.trim();
        var hiddenIcon = hiddenIconInput.text.trim();

        if (visibleIcon !== "" && hiddenIcon !== "") {
            for (var i = 0; i < inputs.length; i++) {
                inputs[i].setToggleIcons(visibleIcon, hiddenIcon);
            }
        }
    }

    // 更新状态
    function updateStatus(message) {
        statusLabel.text = message;
    }

    // 组件加载完成后的初始化
    Component.onCompleted: {
        updateStatus(qsTr("Application started successfully"));
    }
}
