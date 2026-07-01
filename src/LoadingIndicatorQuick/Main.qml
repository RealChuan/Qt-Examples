import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

// iOS 风格的加载指示器 Demo，与 CircularProgressQuick 布局一致。
ApplicationWindow {
    id: mainWindow
    width: 800
    height: 550
    visible: true
    title: qsTr("Loading Indicator Quick Example")

    // === 状态 ===
    property string statusText: qsTr("Ready")
    property string currentColorTarget: "" // "animation" | "background" | "text"

    // === 颜色对话框 ===
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")

        onAccepted: {
            if (!selectedColor)
                return;

            switch (mainWindow.currentColorTarget) {
            case "animation":
                loadingIndicator.color = selectedColor;
                dotColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            case "background":
                loadingIndicator.backgroundColor = selectedColor;
                bgColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            case "text":
                loadingIndicator.textColor = selectedColor;
                textColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            }
            mainWindow.currentColorTarget = "";
            statusLabel.text = qsTr("Color changed");
        }
    }

    // === 自动隐藏定时器 ===
    Timer {
        id: autoHideTimer
        interval: 5000
        repeat: false
        onTriggered: {
            loadingIndicator.hideOverlay();
            statusLabel.text = qsTr("Loading indicator auto hidden");
            statusLabel.color = "lightyellow";
        }
    }

    // === 主布局 (与 CircularProgressQuick 一致) ===
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // === 左侧: 测试区域 + 加载指示器 (占大部分空间) ===
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 500
                spacing: 10

                Rectangle {
                    id: testArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"
                    border.color: "lightgray"
                    border.width: 1
                    radius: 4

                    Label {
                        anchors.centerIn: parent
                        text: qsTr("Test loading area")
                        font.bold: true
                        color: "gray"
                    }

                    LoadingIndicator {
                        id: loadingIndicator
                        visible: false
                        anchors.fill: parent

                        onTextChanged: {
                            statusLabel.text = qsTr("Text changed to: ") + text;
                            statusLabel.color = "lightgreen";
                        }
                        onTextColorChanged: {
                            statusLabel.text = qsTr("Text color changed");
                        }
                        onColorChanged: {
                            statusLabel.text = qsTr("Animation color changed");
                            statusLabel.color = "lightblue";
                        }
                        onBackgroundColorChanged: {
                            statusLabel.text = qsTr("Background color changed");
                        }
                        onAnimationSpeedChanged: {
                            statusLabel.text = qsTr("Animation speed changed to %1 ms").arg(animationSpeed);
                        }
                        onAnimationStyleChanged: {
                            statusLabel.text = qsTr("Animation style changed");
                        }
                    }
                }
            }

            // === 右侧: 控制面板 (固定宽度) ===
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: 300
                Layout.maximumWidth: 360
                spacing: 10

                // 动画设置组
                GroupBox {
                    title: qsTr("Animation Settings")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: qsTr("Style:")
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
                                        text: qsTr("Custom GIF"),
                                        value: LoadingIndicator.AnimationStyle.CustomMovie
                                    }
                                ]
                                valueRole: "value"
                                textRole: "text"
                                onCurrentValueChanged: {
                                    loadingIndicator.animationStyle = currentValue;
                                    if (currentValue === LoadingIndicator.AnimationStyle.CustomMovie) {
                                        loadingIndicator.setMovie("LoadingIndicator/resources/loading.gif");
                                    }
                                }
                            }
                        }

                        Label {
                            id: speedValueLabel
                            text: qsTr("Animation speed: 100 ms")
                        }

                        Slider {
                            id: speedSlider
                            Layout.fillWidth: true
                            from: 50
                            to: 500
                            value: 100
                            onValueChanged: {
                                speedValueLabel.text = qsTr("Animation speed: %1 ms").arg(Math.round(value));
                                loadingIndicator.animationSpeed = Math.round(value);
                            }
                        }
                    }
                }

                // 文本设置组
                GroupBox {
                    title: qsTr("Text Settings")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            CheckBox {
                                id: showTextCheckbox
                                text: qsTr("Show Text")
                                checked: true
                                onToggled: {
                                    loadingIndicator.text = (checked && textEdit.text !== "") ? textEdit.text : "";
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
                                textFromValue: function (value, locale) {
                                    return qsTr("%1 seconds").arg(value);
                                }
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }
                }

                // 颜色设置组
                GroupBox {
                    title: qsTr("Color settings")
                    Layout.fillWidth: true

                    GridLayout {
                        anchors.fill: parent
                        columns: 2
                        rowSpacing: 6
                        columnSpacing: 10

                        Label {
                            text: qsTr("Animation:")
                        }
                        Button {
                            id: dotColorButton
                            Layout.fillWidth: true
                            text: loadingIndicator.color.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "animation";
                                colorDialog.selectedColor = loadingIndicator.color;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingIndicator.color
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Background:")
                        }
                        Button {
                            id: bgColorButton
                            Layout.fillWidth: true
                            text: loadingIndicator.backgroundColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "background";
                                colorDialog.selectedColor = loadingIndicator.backgroundColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingIndicator.backgroundColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Text:")
                        }
                        Button {
                            id: textColorButton
                            Layout.fillWidth: true
                            text: loadingIndicator.textColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "text";
                                colorDialog.selectedColor = loadingIndicator.textColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingIndicator.textColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }
                    }
                }

                // 操作组
                GroupBox {
                    title: qsTr("Actions")
                    Layout.fillWidth: true

                    GridLayout {
                        anchors.fill: parent
                        columns: 2

                        Button {
                            text: qsTr("Show")
                            Layout.fillWidth: true
                            onClicked: {
                                loadingIndicator.animationStyle = styleComboBox.currentValue;

                                loadingIndicator.animationSpeed = speedSlider.value;

                                if (showTextCheckbox.checked && textEdit.text !== "") {
                                    loadingIndicator.text = textEdit.text;
                                } else {
                                    loadingIndicator.text = "";
                                }

                                loadingIndicator.showOverlay(testArea);
                                statusLabel.text = qsTr("Loading indicator shown");
                                statusLabel.color = "lightgreen";

                                if (autoHideCheckbox.checked) {
                                    autoHideTimer.stop();
                                    autoHideTimer.interval = durationSpinBox.value * 1000;
                                    autoHideTimer.start();
                                }
                            }
                        }

                        Button {
                            text: qsTr("Hide")
                            Layout.fillWidth: true
                            onClicked: {
                                loadingIndicator.hideOverlay();
                                autoHideTimer.stop();
                                statusLabel.text = qsTr("Loading indicator hidden");
                                statusLabel.color = "lightcoral";
                            }
                        }
                    }
                }

                // 快速主题组
                GroupBox {
                    title: qsTr("Quick Themes")
                    Layout.fillWidth: true

                    GridLayout {
                        anchors.fill: parent
                        columns: 3

                        Button {
                            text: qsTr("Classic")
                            Layout.fillWidth: true
                            onClicked: mainWindow.applyClassicTheme()
                        }
                        Button {
                            text: qsTr("Dark")
                            Layout.fillWidth: true
                            onClicked: mainWindow.applyDarkTheme()
                        }
                        Button {
                            text: qsTr("Modern")
                            Layout.fillWidth: true
                            onClicked: mainWindow.applyModernTheme()
                        }
                    }
                }
            }
        }

        // === 底部状态栏 (与 CircularProgressQuick 一致) ===
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Ready")
            horizontalAlignment: Text.AlignHCenter
            padding: 10
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        dotColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.color);
        bgColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.backgroundColor);
        textColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.textColor);
    }

    // === 主题函数 (与 CircularProgressQuick 一致: 命名函数 + 内联颜色) ===

    function applyClassicTheme() {
        loadingIndicator.color = "#007aff";          // #007aff systemBlue
        loadingIndicator.textColor = "#1c1c1e";      // #1c1c1e label
        loadingIndicator.backgroundColor = "#F2F2F7E6"; // #F2F2F7E6
        updateColorButtons();
        statusLabel.text = qsTr("Classic theme applied");
        statusLabel.color = "lightblue";
    }

    function applyDarkTheme() {
        loadingIndicator.color = "#0a84ff";       // #0a84ff systemBlue dark
        loadingIndicator.textColor = "#ffffff";    // white
        loadingIndicator.backgroundColor = "#1c1c1eE6"; // #1c1c1eE6
        updateColorButtons();
        statusLabel.text = qsTr("Dark theme applied");
        statusLabel.color = "darkgray";
    }

    function applyModernTheme() {
        loadingIndicator.color = "#ff9500";       // #ff9500 systemOrange
        loadingIndicator.textColor = "#1c1c1e";    // #1c1c1e
        loadingIndicator.backgroundColor = "#ffffffE6"; // #FFFFFFE6
        updateColorButtons();
        statusLabel.text = qsTr("Modern theme applied");
        statusLabel.color = "lightpink";
    }

    function updateColorButtons() {
        dotColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.color);
        bgColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.backgroundColor);
        textColorButton.palette.buttonText = getContrastTextColor(loadingIndicator.textColor);
    }

    // 根据 WCAG 标准计算对比文字色 (黑或白)
    function getContrastTextColor(bgColor) {
        function normalize(x) {
            return x <= 0.03928 ? x / 12.92 : Math.pow((x + 0.055) / 1.055, 2.4);
        }
        const luminance = 0.2126 * normalize(bgColor.r) + 0.7152 * normalize(bgColor.g) + 0.0722 * normalize(bgColor.b);
        return luminance > 0.179 ? "black" : "white";
    }
}
