import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow

    width: 800
    height: 650
    visible: true
    title: qsTr("Circular Progress Quick Example")

    // 当前正在编辑颜色的目标 (用于 ColorDialog 异步回调)
    property string currentColorTarget: ""

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    // 接受颜色对话框 (替代 arguments.callee 模式)
    Connections {
        target: colorDialog
        function onAccepted() {
            if (!colorDialog.selectedColor)
                return;

            switch (mainWindow.currentColorTarget) {
            case "arc":
                circularProgress.arcColor = colorDialog.selectedColor;
                arcColorButton.palette.buttonText = getContrastTextColor(circularProgress.arcColor);
                break;
            case "text":
                circularProgress.textColor = colorDialog.selectedColor;
                textColorButton.palette.buttonText = getContrastTextColor(circularProgress.textColor);
                break;
            case "title":
                circularProgress.titleColor = colorDialog.selectedColor;
                titleColorButton.palette.buttonText = getContrastTextColor(circularProgress.titleColor);
                break;
            case "base":
                circularProgress.baseColor = colorDialog.selectedColor;
                baseColorButton.palette.buttonText = getContrastTextColor(circularProgress.baseColor);
                break;
            case "background":
                circularProgress.backgroundColor = colorDialog.selectedColor;
                backgroundColorButton.palette.buttonText = getContrastTextColor(circularProgress.backgroundColor);
                break;
            }
            mainWindow.currentColorTarget = "";
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // === 左侧区域 ===
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // 左上角：进度显示
                CircularProgress {
                    id: circularProgress
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 10
                    value: 25
                    title: qsTr("Loading Progress")
                    showPercent: true
                }

                // 数值控制组
                GroupBox {
                    title: qsTr("Value Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 3
                        anchors.fill: parent

                        Label {
                            text: qsTr("Current value:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Slider {
                            id: valueSlider
                            Layout.fillWidth: true
                            from: circularProgress.minValue
                            to: circularProgress.maxValue
                            value: circularProgress.value
                            onMoved: {
                                if (pressed) {
                                    circularProgress.setValue(value);
                                }
                            }
                        }

                        SpinBox {
                            id: valueSpinBox
                            from: circularProgress.minValue
                            to: circularProgress.maxValue
                            value: circularProgress.value
                            editable: true
                            onValueModified: circularProgress.setValue(value)
                        }

                        Label {
                            text: qsTr("Min value:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: minValueSpinBox
                            from: -1000
                            to: 1000
                            value: circularProgress.minValue
                            editable: true
                            onValueModified: circularProgress.minValue = value
                            Layout.columnSpan: 2
                        }

                        Label {
                            text: qsTr("Max value:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: maxValueSpinBox
                            from: -1000
                            to: 1000
                            value: circularProgress.maxValue
                            editable: true
                            onValueModified: circularProgress.maxValue = value
                            Layout.columnSpan: 2
                        }

                        CheckBox {
                            id: showPercentCheckbox
                            text: qsTr("Show percentage")
                            checked: circularProgress.showPercent
                            onToggled: circularProgress.showPercent = checked
                            Layout.columnSpan: 3
                        }
                    }
                }

                // 文本设置组
                GroupBox {
                    title: qsTr("Text Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        anchors.fill: parent

                        Label {
                            text: qsTr("Title:")
                            Layout.alignment: Qt.AlignRight
                        }

                        TextField {
                            id: titleTextField
                            Layout.fillWidth: true
                            text: circularProgress.title
                            onTextChanged: circularProgress.title = text
                            placeholderText: qsTr("Enter progress title")
                        }
                    }
                }
            }

            // === 右侧：控制面板 ===
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // 几何设置组
                GroupBox {
                    title: qsTr("Geometry Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        anchors.fill: parent

                        Label {
                            text: qsTr("Start angle:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: startAngleSpinBox
                            from: 0
                            to: 360
                            value: circularProgress.startAngle
                            editable: true
                            onValueModified: circularProgress.startAngle = value
                        }

                        Label {
                            text: qsTr("End angle:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: endAngleSpinBox
                            from: startAngleSpinBox.value
                            to: 720
                            value: circularProgress.endAngle
                            editable: true
                            onValueModified: circularProgress.endAngle = value
                        }
                    }
                }

                // 颜色设置组
                GroupBox {
                    title: qsTr("Color Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        anchors.fill: parent

                        // 第一列
                        Label {
                            text: qsTr("Arc color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: arcColorButton
                            Layout.fillWidth: true
                            text: circularProgress.arcColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = circularProgress.arcColor;
                                mainWindow.currentColorTarget = "arc";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: circularProgress.arcColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Text color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: textColorButton
                            Layout.fillWidth: true
                            text: circularProgress.textColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = circularProgress.textColor;
                                mainWindow.currentColorTarget = "text";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: circularProgress.textColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Title color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: titleColorButton
                            Layout.fillWidth: true
                            text: circularProgress.titleColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = circularProgress.titleColor;
                                mainWindow.currentColorTarget = "title";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: circularProgress.titleColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        // 第二列
                        Label {
                            text: qsTr("Base color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: baseColorButton
                            Layout.fillWidth: true
                            text: circularProgress.baseColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = circularProgress.baseColor;
                                mainWindow.currentColorTarget = "base";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: circularProgress.baseColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Background color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: backgroundColorButton
                            Layout.fillWidth: true
                            text: circularProgress.backgroundColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = circularProgress.backgroundColor;
                                mainWindow.currentColorTarget = "background";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: circularProgress.backgroundColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }
                    }
                }

                // 动画设置组
                GroupBox {
                    title: qsTr("Animation Settings")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent

                        Label {
                            id: durationLabel
                            text: qsTr("Animation duration: %1ms").arg(circularProgress.animationDuration)
                            Layout.fillWidth: true
                        }

                        Slider {
                            id: animationDurationSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 2000
                            value: circularProgress.animationDuration
                            onValueChanged: {
                                circularProgress.animationDuration = value;
                                durationLabel.text = qsTr("Animation duration: %1ms").arg(value);
                            }
                        }
                    }
                }

                // 操作按钮组
                GroupBox {
                    title: qsTr("Actions")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        anchors.fill: parent

                        Button {
                            text: qsTr("+10")
                            Layout.fillWidth: true
                            onClicked: circularProgress.increaseValue(10)
                        }

                        Button {
                            text: qsTr("-10")
                            Layout.fillWidth: true
                            onClicked: circularProgress.decreaseValue(10)
                        }

                        Button {
                            text: qsTr("Reset")
                            Layout.fillWidth: true
                            onClicked: circularProgress.reset()
                        }

                        Button {
                            text: qsTr("Animate to 75")
                            Layout.fillWidth: true
                            onClicked: circularProgress.setValueAnimated(75)
                        }
                    }
                }

                // 快速主题组 (iOS 风格调色板, 与 QWidget 版本一致)
                GroupBox {
                    title: qsTr("Quick Themes")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 3
                        anchors.fill: parent

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

        // === 底部状态显示 ===
        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Ready")
            horizontalAlignment: Text.AlignHCenter
            padding: 10
        }
    }

    // === 信号连接 ===
    Connections {
        target: circularProgress

        function onValueIncreased(newValue: real) {
            statusLabel.text = qsTr("Value increased to: %1").arg(newValue);
            statusLabel.background.color = "lightgreen";
        }

        function onValueDecreased(newValue: real) {
            statusLabel.text = qsTr("Value decreased to: %1").arg(newValue);
            statusLabel.background.color = "lightcoral";
        }

        function onValueReset() {
            statusLabel.text = qsTr("Value reset to minimum");
            statusLabel.background.color = "lightyellow";
        }

        function onAnimationStarted(oldValue: real, newValue: real) {
            statusLabel.text = qsTr("Animating: %1 → %2").arg(oldValue).arg(newValue);
            statusLabel.background.color = "lightblue";
        }

        function onAnimationFinished(value: real) {
            statusLabel.text = qsTr("Animation finished: %1").arg(value);
            statusLabel.background.color = "lightgreen";
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        updateColorButtons();
    }

    // === 工具函数 ===

    // 计算对比度足够的文本颜色 (WCAG 标准)
    function getContrastTextColor(bgColor: color): color {
        const normalize = x => x <= 0.03928 ? x / 12.92 : Math.pow((x + 0.055) / 1.055, 2.4);
        const getRelativeLuminance = (r, g, b) => 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0) + 0.0722 * normalize(b / 255.0);

        const luminance = getRelativeLuminance(bgColor.r * 255, bgColor.g * 255, bgColor.b * 255);
        return luminance > 0.179 ? "black" : "white";
    }

    // 更新所有颜色按钮的文本颜色
    function updateColorButtons() {
        arcColorButton.palette.buttonText = getContrastTextColor(circularProgress.arcColor);
        textColorButton.palette.buttonText = getContrastTextColor(circularProgress.textColor);
        titleColorButton.palette.buttonText = getContrastTextColor(circularProgress.titleColor);
        baseColorButton.palette.buttonText = getContrastTextColor(circularProgress.baseColor);
        backgroundColorButton.palette.buttonText = getContrastTextColor(circularProgress.backgroundColor);
    }

    // === 主题函数 (iOS 风格调色板, 与 QWidget 版本一致) ===

    function applyClassicTheme() {
        circularProgress.arcColor = "#007aff";
        circularProgress.textColor = "#1c1c1e";
        circularProgress.titleColor = "#8e8e93";
        circularProgress.baseColor = "#d1d1d6";
        circularProgress.backgroundColor = "transparent";

        updateColorButtons();
        statusLabel.text = qsTr("Classic theme applied");
        statusLabel.color = "lightblue";
    }

    function applyDarkTheme() {
        circularProgress.arcColor = "#0a84ff";
        circularProgress.textColor = "#ffffff";
        circularProgress.titleColor = "#aeaeb2";
        circularProgress.baseColor = "#48484a";
        circularProgress.backgroundColor = "#1c1c1e";

        updateColorButtons();
        statusLabel.text = qsTr("Dark theme applied");
        statusLabel.color = "darkgray";
    }

    function applyModernTheme() {
        circularProgress.arcColor = "#ff9500";
        circularProgress.textColor = "#1c1c1e";
        circularProgress.titleColor = "#ff2d55";
        circularProgress.baseColor = "#c7c7cc";
        circularProgress.backgroundColor = "#f2f2f7";

        updateColorButtons();
        statusLabel.text = qsTr("Modern theme applied");
        statusLabel.color = "lightpink";
    }
}
