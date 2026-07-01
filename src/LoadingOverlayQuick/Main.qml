import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

// iOS 风格的加载浮层 Demo，与 LoadingIndicatorQuick 布局一致。
ApplicationWindow {
    id: mainWindow
    width: 380
    height: 500
    visible: true
    title: qsTr("Loading Overlay Quick Example")

    // === 状态 ===
    property string statusText: qsTr("Ready")
    property string currentColorTarget: "" // "animation" | "card" | "text" | "dim"

    // === 颜色对话框 ===
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")

        onAccepted: {
            if (!selectedColor)
                return;

            switch (mainWindow.currentColorTarget) {
            case "animation":
                loadingOverlay.animationColor = selectedColor;
                animColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            case "card":
                loadingOverlay.cardBackgroundColor = selectedColor;
                cardColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            case "text":
                loadingOverlay.textColor = selectedColor;
                textColorButton.palette.buttonText = getContrastTextColor(selectedColor);
                break;
            case "dim":
                loadingOverlay.dimColor = selectedColor;
                dimColorButton.palette.buttonText = getContrastTextColor(selectedColor);
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
            loadingOverlay.hide();
            statusLabel.text = qsTr("Loading overlay auto hidden");
            statusLabel.color = "lightyellow";
        }
    }

    // === 主布局 (与 LoadingIndicatorQuick 一致) ===
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

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
                                loadingOverlay.animationSpeed = Math.round(value);
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
                                    loadingOverlay.text = (checked && textEdit.text !== "") ? textEdit.text : "";
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
                            id: animColorButton
                            Layout.fillWidth: true
                            text: loadingOverlay.animationColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "animation";
                                colorDialog.selectedColor = loadingOverlay.animationColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingOverlay.animationColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Card:")
                        }
                        Button {
                            id: cardColorButton
                            Layout.fillWidth: true
                            text: loadingOverlay.cardBackgroundColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "card";
                                colorDialog.selectedColor = loadingOverlay.cardBackgroundColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingOverlay.cardBackgroundColor
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
                            text: loadingOverlay.textColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "text";
                                colorDialog.selectedColor = loadingOverlay.textColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingOverlay.textColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Dim:")
                        }
                        Button {
                            id: dimColorButton
                            Layout.fillWidth: true
                            text: loadingOverlay.dimColor.toString().toUpperCase()
                            onClicked: {
                                mainWindow.currentColorTarget = "dim";
                                colorDialog.selectedColor = loadingOverlay.dimColor;
                                colorDialog.open();
                            }
                            background: Rectangle {
                                color: loadingOverlay.dimColor
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
                                if (showTextCheckbox.checked && textEdit.text !== "") {
                                    loadingOverlay.text = textEdit.text;
                                } else {
                                    loadingOverlay.text = "";
                                }
                                loadingOverlay.show(loadingOverlay.text);
                                statusLabel.text = qsTr("Loading overlay shown");
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
                                loadingOverlay.hide();
                                autoHideTimer.stop();
                                statusLabel.text = qsTr("Loading overlay hidden");
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

    // 加载浮层
    LoadingOverlay {
        id: loadingOverlay

        onTextChanged: {
            statusLabel.text = qsTr("Text changed to: ") + loadingOverlay.text;
        }
        onTextColorChanged: {
            statusLabel.text = qsTr("Text color changed");
        }
        onAnimationColorChanged: {
            statusLabel.text = qsTr("Animation color changed");
            statusLabel.color = "lightblue";
        }
        onCardBackgroundColorChanged: {
            statusLabel.text = qsTr("Card color changed");
        }
        onDimColorChanged: {
            statusLabel.text = qsTr("Dim color changed");
        }
        onAnimationSpeedChanged: {
            statusLabel.text = qsTr("Animation speed changed to %1 ms").arg(animationSpeed);
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        animColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.animationColor);
        cardColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.cardBackgroundColor);
        textColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.textColor);
        dimColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.dimColor);
    }

    // === 主题函数 (与 CircularProgressQuick 一致: 命名函数 + 内联颜色) ===

    function applyClassicTheme() {
        loadingOverlay.animationColor = "#007aff";        // #007aff systemBlue
        loadingOverlay.textColor = "#1c1c1e";            // #1c1c1e label
        loadingOverlay.cardBackgroundColor = "#F2F2F7F2"; // #F2F2F7F2
        loadingOverlay.dimColor = "#00000066";           // 40% black
        updateColorButtons();
        statusLabel.text = qsTr("Classic theme applied");
        statusLabel.color = "lightblue";
    }

    function applyDarkTheme() {
        loadingOverlay.animationColor = "#0a84ff";       // #0a84ff systemBlue dark
        loadingOverlay.textColor = "#ffffff";            // white
        loadingOverlay.cardBackgroundColor = "#1c1c1ef2"; // #1c1c1ef2
        loadingOverlay.dimColor = "#00000099";           // 60% black
        updateColorButtons();
        statusLabel.text = qsTr("Dark theme applied");
        statusLabel.color = "darkgray";
    }

    function applyModernTheme() {
        loadingOverlay.animationColor = "#ff9500";       // #ff9500 systemOrange
        loadingOverlay.textColor = "#1c1c1e";            // #1c1c1e
        loadingOverlay.cardBackgroundColor = "#fffffff2"; // #fffffff2
        loadingOverlay.dimColor = "#00000033";           // 20% black
        updateColorButtons();
        statusLabel.text = qsTr("Modern theme applied");
        statusLabel.color = "lightpink";
    }

    function updateColorButtons() {
        animColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.animationColor);
        cardColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.cardBackgroundColor);
        textColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.textColor);
        dimColorButton.palette.buttonText = getContrastTextColor(loadingOverlay.dimColor);
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
