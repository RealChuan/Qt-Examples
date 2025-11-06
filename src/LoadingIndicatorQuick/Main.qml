import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow

    width: 450
    height: 550
    visible: true
    title: qsTr("Loading Indicator Quick Example")

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
            loadingIndicator.hideOverlay();
            mainWindow.statusText = qsTr("Loading indicator auto hidden");
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // 显示/隐藏按钮
        RowLayout {
            Layout.fillWidth: true
            Button {
                text: qsTr("Show Loading Indicator")
                Layout.fillWidth: true
                onClicked: {
                    // 设置动画样式
                    loadingIndicator.animationStyle = styleComboBox.currentValue;

                    // 设置动画速度
                    loadingIndicator.animationSpeed = speedSlider.value;

                    // 设置文本
                    if (showTextCheckbox.checked && textEdit.text !== "") {
                        loadingIndicator.text = textEdit.text;
                    } else {
                        loadingIndicator.text = "";
                    }

                    // 显示加载指示器
                    loadingIndicator.showOverlay(testArea);

                    // 更新状态
                    mainWindow.statusText = qsTr("Loading indicator shown");

                    // 设置自动隐藏
                    if (autoHideCheckbox.checked) {
                        autoHideTimer.stop();
                        autoHideTimer.interval = durationSpinBox.value * 1000;
                        autoHideTimer.start();
                    }
                }
            }
            Button {
                text: qsTr("Hide Loading Indicator")
                Layout.fillWidth: true
                onClicked: {
                    loadingIndicator.hideOverlay();
                    autoHideTimer.stop();
                    mainWindow.statusText = qsTr("Loading indicator hidden");
                }
            }
        }

        // 动画样式选择
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Animation Style:")
                Layout.preferredWidth: 120
            }
            ComboBox {
                id: styleComboBox
                Layout.fillWidth: true
                model: [
                    {
                        text: qsTr("Rotating Dots"),
                        value: LoadingIndicator.AnimationStyle.RotatingDots
                    },
                    {
                        text: qsTr("Pulsing Circle"),
                        value: LoadingIndicator.AnimationStyle.PulsingCircle
                    },
                    {
                        text: qsTr("Bouncing Bars"),
                        value: LoadingIndicator.AnimationStyle.BouncingBars
                    },
                    {
                        text: qsTr("Custom Movie"),
                        value: LoadingIndicator.AnimationStyle.CustomMovie
                    }
                ]
                valueRole: "value"
                textRole: "text"
                onCurrentValueChanged: {
                    loadingIndicator.animationStyle = currentValue ? currentValue : 0;

                    // 如果选择了 CustomMovie，设置默认的 GIF 源
                    if (currentValue === LoadingIndicator.AnimationStyle.CustomMovie) {
                        loadingIndicator.setMovie("LoadingIndicator/resources/loading.gif");
                    }
                }
            }
        }

        // 动画速度控制
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: qsTr("Animation Speed:")
                Layout.preferredWidth: 120
            }
            Slider {
                id: speedSlider
                Layout.fillWidth: true
                from: 50
                to: 500
                value: 100
                onValueChanged: {
                    speedValueLabel.text = Math.round(value) + " ms";
                    loadingIndicator.animationSpeed = Math.round(value);
                }
            }
            Label {
                id: speedValueLabel
                text: "100 ms"
                Layout.preferredWidth: 60
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
                        loadingIndicator.text = textEdit.text;
                    } else {
                        loadingIndicator.text = "";
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
                        loadingIndicator.text = text;
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
                text: qsTr("Animation Color:")
            }

            Button {
                id: dotColorButton
                Layout.fillWidth: true
                text: loadingIndicator.color.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = loadingIndicator.color;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            loadingIndicator.color = colorDialog.selectedColor;
                            dotColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.color);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: loadingIndicator.color
                    border.color: "gray"
                    border.width: 1
                }
            }

            Label {
                text: qsTr("Background Color:")
            }
            Button {
                id: bgColorButton
                Layout.fillWidth: true
                text: loadingIndicator.backgroundColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = loadingIndicator.backgroundColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            loadingIndicator.backgroundColor = colorDialog.selectedColor;
                            bgColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.backgroundColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: loadingIndicator.backgroundColor
                    border.color: "gray"
                    border.width: 1
                }
            }

            Label {
                text: qsTr("Text Color:")
            }
            Button {
                id: textColorButton
                Layout.fillWidth: true
                text: loadingIndicator.textColor.toString().toUpperCase()
                onClicked: {
                    colorDialog.selectedColor = loadingIndicator.textColor;
                    colorDialog.accepted.connect(function () {
                        if (colorDialog.selectedColor) {
                            loadingIndicator.textColor = colorDialog.selectedColor;
                            textColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.textColor);
                        }
                        colorDialog.accepted.disconnect(arguments.callee);
                    });
                    colorDialog.open();
                }

                background: Rectangle {
                    color: loadingIndicator.textColor
                    border.color: "gray"
                    border.width: 1
                }
            }
        }

        // 状态显示
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "transparent"
            border.color: "gray"
            border.width: 1

            Label {
                anchors.centerIn: parent
                text: mainWindow.statusText
                font.bold: true
            }
        }

        // 测试区域
        Rectangle {
            id: testArea
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            color: "transparent"
            border.color: "darkgray"
            border.width: 1

            Label {
                anchors.centerIn: parent
                text: qsTr("Test loading area")
                font.bold: true
            }

            // 加载指示器
            LoadingIndicator {
                id: loadingIndicator
                visible: false
                anchors.fill: parent

                // 监听属性变化
                onTextChanged: mainWindow.statusText = qsTr("Text changed to: ") + loadingIndicator.text
                onTextColorChanged: {
                    mainWindow.currentTextColor = loadingIndicator.textColor;
                    mainWindow.statusText = qsTr("Text color changed: ") + loadingIndicator.textColor;
                }
                onColorChanged: {
                    mainWindow.statusText = qsTr("Animation color changed: ") + color;
                }
                onBackgroundColorChanged: {
                    mainWindow.statusText = qsTr("Background color changed: ") + backgroundColor;
                }
                onAnimationSpeedChanged: {
                    mainWindow.statusText = qsTr("Animation speed changed to %1 ms").arg(animationSpeed);
                }
                onAnimationStyleChanged: {
                    mainWindow.statusText = qsTr("Animation style changed");
                }
            }
        }
    }

    Component.onCompleted: {
        dotColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.color);
        bgColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.backgroundColor);
        textColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.textColor);
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
