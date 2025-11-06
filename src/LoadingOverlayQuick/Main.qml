import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 360
    height: 220
    visible: true
    title: qsTr("Loading Overlay Quick Example")

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    // 状态文本
    property string statusText: qsTr("Ready")

    // 自动隐藏定时器
    Timer {
        id: autoHideTimer
        interval: 5000 // 5秒
        repeat: false
        onTriggered: {
            loadingOverlay.hide();
            mainWindow.statusText = qsTr("Loading overlay auto hidden");
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 显示按钮
        Button {
            text: qsTr("Show Loading Overlay")
            Layout.fillWidth: true
            onClicked: {
                // 设置文本
                if (showTextCheckbox.checked && textEdit.text !== "") {
                    loadingOverlay.text = textEdit.text;
                } else {
                    loadingOverlay.text = "";
                }

                // 显示加载层
                loadingOverlay.show(textEdit.text);

                // 更新状态
                mainWindow.statusText = qsTr("Loading overlay shown");

                // 设置自动隐藏
                if (autoHideCheckbox.checked) {
                    autoHideTimer.stop();
                    autoHideTimer.interval = durationSpinBox.value * 1000;
                    autoHideTimer.start();
                }
            }
        }

        // 文本设置
        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                id: showTextCheckbox
                text: qsTr("Show Text")
                checked: true
                onToggled: {
                    if (checked && textEdit.text !== "") {
                        loadingOverlay.text = textEdit.text;
                    } else {
                        loadingOverlay.text = "";
                    }
                }
            }
            TextField {
                id: textEdit
                Layout.fillWidth: true
                placeholderText: qsTr("Enter loading text")
                text: qsTr("Loading...")
                onTextChanged: {
                    if (showTextCheckbox.checked) {
                        loadingOverlay.text = text;
                    }
                }
            }
        }

        // 自动隐藏设置
        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                id: autoHideCheckbox
                text: qsTr("Auto Hide After:")
                checked: true
            }
            SpinBox {
                id: durationSpinBox
                from: 1
                to: 60
                value: 5
            }
            Label {
                text: qsTr("seconds")
            }
            Item {
                Layout.fillWidth: true
            }
        }

        // 颜色设置
        GridLayout {
            Layout.fillWidth: true
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            Label {
                text: qsTr("Text Color:")
            }
            Button {
                id: textColorButton
                Layout.fillWidth: true
                text: loadingOverlay.textColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = loadingOverlay.textColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            loadingOverlay.textColor = colorDialog.selectedColor;
                            textColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.textColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: loadingOverlay.textColor
                    border.color: "gray"
                    border.width: 1
                }
            }
        }

        // 状态显示
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"
            border.color: "gray"
            border.width: 1

            Label {
                anchors.centerIn: parent
                text: mainWindow.statusText
                font.bold: true
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // 加载层 - 覆盖整个窗口
    LoadingOverlay {
        id: loadingOverlay

        // 监听属性变化
        onTextChanged: mainWindow.statusText = qsTr("Text changed to: ") + loadingOverlay.text
        onTextColorChanged: {
            mainWindow.statusText = qsTr("Text color changed: ") + textColor;
        }
    }

    Component.onCompleted: {
        textColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.textColor);
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
