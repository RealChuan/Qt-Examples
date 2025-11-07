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

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
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

                        Label {
                            text: qsTr("Arc width:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: arcWidthSpinBox
                            from: 5
                            to: 50
                            value: circularProgress.arcWidth
                            editable: true
                            onValueModified: circularProgress.arcWidth = value
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
                                colorDialog.accepted.connect(function () {
                                    if (colorDialog.selectedColor) {
                                        circularProgress.arcColor = colorDialog.selectedColor;
                                        arcColorButton.palette.buttonText = getContrastTextColor(circularProgress.arcColor);
                                    }
                                    colorDialog.accepted.disconnect(arguments.callee);
                                });
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
                                colorDialog.accepted.connect(function () {
                                    if (colorDialog.selectedColor) {
                                        circularProgress.textColor = colorDialog.selectedColor;
                                        textColorButton.palette.buttonText = getContrastTextColor(circularProgress.textColor);
                                    }
                                    colorDialog.accepted.disconnect(arguments.callee);
                                });
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
                                colorDialog.accepted.connect(function () {
                                    if (colorDialog.selectedColor) {
                                        circularProgress.titleColor = colorDialog.selectedColor;
                                        titleColorButton.palette.buttonText = getContrastTextColor(circularProgress.titleColor);
                                    }
                                    colorDialog.accepted.disconnect(arguments.callee);
                                });
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
                                colorDialog.accepted.connect(function () {
                                    if (colorDialog.selectedColor) {
                                        circularProgress.baseColor = colorDialog.selectedColor;
                                        baseColorButton.palette.buttonText = getContrastTextColor(circularProgress.baseColor);
                                    }
                                    colorDialog.accepted.disconnect(arguments.callee);
                                });
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
                                colorDialog.accepted.connect(function () {
                                    if (colorDialog.selectedColor) {
                                        circularProgress.backgroundColor = colorDialog.selectedColor;
                                        backgroundColorButton.palette.buttonText = getContrastTextColor(circularProgress.backgroundColor);
                                    }
                                    colorDialog.accepted.disconnect(arguments.callee);
                                });
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

                // 快速主题组
                GroupBox {
                    title: qsTr("Quick Themes")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
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

                        Button {
                            text: qsTr("Reset Colors")
                            Layout.fillWidth: true
                            onClicked: mainWindow.resetColors()
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

        function onValueIncreased(newValue) {
            console.log("Value increased to:", newValue);
            statusLabel.text = qsTr("Value increased to: %1").arg(newValue);
            statusLabel.background.color = "lightgreen";
        }

        function onValueDecreased(newValue) {
            console.log("Value decreased to:", newValue);
            statusLabel.text = qsTr("Value decreased to: %1").arg(newValue);
            statusLabel.background.color = "lightcoral";
        }

        function onValueReset() {
            console.log("Value reset");
            statusLabel.text = qsTr("Value reset to minimum");
            statusLabel.background.color = "lightyellow";
        }

        function onAnimationStarted(oldValue, newValue) {
            console.log("Animation started from", oldValue, "to", newValue);
            statusLabel.text = qsTr("Animating: %1 → %2").arg(oldValue).arg(newValue);
            statusLabel.background.color = "lightblue";
        }

        function onAnimationFinished(value) {
            console.log("Animation finished at:", value);
            statusLabel.text = qsTr("Animation finished: %1").arg(value);
            statusLabel.background.color = "lightgreen";
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        updateColorButtons();
    }

    // === 工具函数 ===

    // 计算对比度足够的文本颜色（WCAG标准）
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

    // 更新所有颜色按钮的文本和颜色
    function updateColorButtons() {
        arcColorButton.palette.buttonText = getContrastTextColor(circularProgress.arcColor);
        textColorButton.palette.buttonText = getContrastTextColor(circularProgress.textColor);
        titleColorButton.palette.buttonText = getContrastTextColor(circularProgress.titleColor);
        baseColorButton.palette.buttonText = getContrastTextColor(circularProgress.baseColor);
        backgroundColorButton.palette.buttonText = getContrastTextColor(circularProgress.backgroundColor);
    }

    // === 主题函数 ===

    function applyClassicTheme() {
        circularProgress.arcColor = "#4da1ff";
        circularProgress.textColor = "#4da1ff";
        circularProgress.titleColor = "#505050";
        circularProgress.baseColor = "#b3b3b3";
        circularProgress.backgroundColor = "transparent";

        updateColorButtons();
        statusLabel.text = qsTr("Classic theme applied");
        statusLabel.background.color = "lightblue";
    }

    function applyDarkTheme() {
        circularProgress.arcColor = "#61afef";
        circularProgress.textColor = "#98c379";
        circularProgress.titleColor = "#e06c75";
        circularProgress.baseColor = "#3c4048";
        circularProgress.backgroundColor = "#21252b";

        updateColorButtons();
        statusLabel.text = qsTr("Dark theme applied");
        statusLabel.background.color = "darkgray";
    }

    function applyModernTheme() {
        circularProgress.arcColor = "#dc143c";
        circularProgress.textColor = "#191970";
        circularProgress.titleColor = "#4169e1";
        circularProgress.baseColor = "#d3d3d3";
        circularProgress.backgroundColor = "#f0f8ff";

        updateColorButtons();
        statusLabel.text = qsTr("Modern theme applied");
        statusLabel.background.color = "lightpink";
    }

    function resetColors() {
        circularProgress.arcColor = "#4da1ff";
        circularProgress.textColor = "#4da1ff";
        circularProgress.titleColor = "#505050";
        circularProgress.baseColor = "#b3b3b3";
        circularProgress.backgroundColor = "transparent";

        updateColorButtons();

        statusLabel.text = qsTr("Colors reset to default");
        statusLabel.background.color = "lightyellow";
    }
}
