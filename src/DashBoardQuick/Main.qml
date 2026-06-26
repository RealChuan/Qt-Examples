import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 620
    visible: true
    title: qsTr("DashBoard Quick Example")

    // 当前颜色编辑目标（用于 ColorDialog 回填）
    property string currentColorTarget: ""

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    // 计算对比度足够的文本颜色（WCAG 标准）
    function getContrastTextColor(bgColor: color): color {
        function normalize(x: real): real {
            return x <= 0.03928 ? x / 12.92 : Math.pow((x + 0.055) / 1.055, 2.4);
        }

        function getRelativeLuminance(r: real, g: real, b: real): real {
            return 0.2126 * normalize(r / 255.0) + 0.7152 * normalize(g / 255.0) + 0.0722 * normalize(b / 255.0);
        }

        const luminance = getRelativeLuminance(bgColor.r * 255, bgColor.g * 255, bgColor.b * 255);
        return luminance > 0.179 ? "black" : "white";
    }

    // 初始化颜色按钮文字色
    function initializeColorButtons() {
        arcColorButton.palette.buttonText = getContrastTextColor(dashboard.arcColor);
        scaleColorButton.palette.buttonText = getContrastTextColor(dashboard.scaleColor);
        pointerColorButton.palette.buttonText = getContrastTextColor(dashboard.pointerColor);
        scaleTextColorButton.palette.buttonText = getContrastTextColor(dashboard.scaleTextColor);
        backgroundColorButton.palette.buttonText = getContrastTextColor(dashboard.backgroundColor);
        valueColorButton.palette.buttonText = getContrastTextColor(dashboard.valueColor);
        titleColorButton.palette.buttonText = getContrastTextColor(dashboard.titleColor);
    }

    // === 三套主题（与 QWidget 版本完全一致，明暗区分明显）===
    function applyClassicTheme() {
        dashboard.arcColor = "#383d4a";
        dashboard.scaleColor = "#04a8ad";
        dashboard.pointerColor = "#04b5c8";
        dashboard.scaleTextColor = "#908574";
        dashboard.backgroundColor = "transparent";
        dashboard.valueColor = "#908574";
        dashboard.titleColor = "#908574";
        initializeColorButtons();
    }

    function applyDarkTheme() {
        dashboard.arcColor = "#282c34";
        dashboard.scaleColor = "#61afef";
        dashboard.pointerColor = "#c678dd";
        dashboard.scaleTextColor = "#abb2bf";
        dashboard.backgroundColor = "#1e2127";
        dashboard.valueColor = "#98c379";
        dashboard.titleColor = "#e06c75";
        initializeColorButtons();
    }

    function applyModernTheme() {
        dashboard.arcColor = "#4682b4";
        dashboard.scaleColor = "#6495ed";
        dashboard.pointerColor = "#dc143c";
        dashboard.scaleTextColor = "#2f4f4f";
        dashboard.backgroundColor = "#f0f8ff";
        dashboard.valueColor = "#191970";
        dashboard.titleColor = "#4169e1";
        initializeColorButtons();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15

            // === 左侧：仪表盘显示区域 ===
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // 仪表盘显示（默认 Classic 主题，背景 transparent）
                DashBoard {
                    id: dashboard
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 10
                    value: 25
                    title: qsTr("SPEED")
                    unit: qsTr("km/h")
                }

                // 数值控制组
                GroupBox {
                    title: qsTr("Value Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 3
                        anchors.fill: parent

                        Label {
                            id: valueLabel
                            text: qsTr("Current value:")
                            Layout.alignment: Qt.AlignHCenter
                        }

                        Slider {
                            id: valueSlider
                            Layout.fillWidth: true
                            from: dashboard.minValue
                            to: dashboard.maxValue
                            value: dashboard.value
                            onMoved: {
                                if (pressed) {
                                    dashboard.setValue(value);
                                }
                            }
                        }

                        SpinBox {
                            id: valueSpinBox
                            from: dashboard.minValue
                            to: dashboard.maxValue
                            value: dashboard.value
                            editable: true
                            onValueModified: dashboard.setValue(value)
                        }

                        Label {
                            text: qsTr("Min value:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: minValueSpinBox
                            from: -1000
                            to: 1000
                            value: dashboard.minValue
                            editable: true
                            onValueModified: dashboard.minValue = value
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
                            value: dashboard.maxValue
                            editable: true
                            onValueModified: dashboard.maxValue = value
                            Layout.columnSpan: 2
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
                            text: dashboard.title
                            onTextChanged: dashboard.title = text
                            placeholderText: qsTr("Enter dashboard title")
                        }

                        Label {
                            text: qsTr("Unit:")
                            Layout.alignment: Qt.AlignRight
                        }

                        TextField {
                            id: unitTextField
                            Layout.fillWidth: true
                            text: dashboard.unit
                            onTextChanged: dashboard.unit = text
                            placeholderText: qsTr("Enter value unit")
                        }
                    }
                }
            }

            // === 右侧：控制面板 ===
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 10

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
                            value: dashboard.startAngle
                            editable: true
                            onValueModified: dashboard.startAngle = value
                        }

                        Label {
                            text: qsTr("End angle:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: endAngleSpinBox
                            from: startAngleSpinBox.value
                            to: 720
                            value: dashboard.endAngle
                            editable: true
                            onValueModified: dashboard.endAngle = value
                        }

                        Label {
                            text: qsTr("Major scales:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: scaleMajorSpinBox
                            from: 1
                            to: 100
                            value: dashboard.scaleMajor
                            editable: true
                            onValueModified: dashboard.scaleMajor = value
                        }

                        Label {
                            text: qsTr("Minor scales:")
                            Layout.alignment: Qt.AlignRight
                        }

                        SpinBox {
                            id: scaleMinorSpinBox
                            from: 1
                            to: 100
                            value: dashboard.scaleMinor
                            editable: true
                            onValueModified: dashboard.scaleMinor = value
                        }
                    }
                }

                // 颜色设置组
                GroupBox {
                    title: qsTr("Color Settings")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 4
                        anchors.fill: parent

                        Label {
                            text: qsTr("Arc color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: arcColorButton
                            Layout.fillWidth: true
                            text: dashboard.arcColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.arcColor;
                                mainWindow.currentColorTarget = "arcColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.arcColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Scale color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: scaleColorButton
                            Layout.fillWidth: true
                            text: dashboard.scaleColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.scaleColor;
                                mainWindow.currentColorTarget = "scaleColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.scaleColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Pointer color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: pointerColorButton
                            Layout.fillWidth: true
                            text: dashboard.pointerColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.pointerColor;
                                mainWindow.currentColorTarget = "pointerColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.pointerColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Text color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: scaleTextColorButton
                            Layout.fillWidth: true
                            text: dashboard.scaleTextColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.scaleTextColor;
                                mainWindow.currentColorTarget = "scaleTextColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.scaleTextColor
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
                            text: dashboard.backgroundColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.backgroundColor;
                                mainWindow.currentColorTarget = "backgroundColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.backgroundColor
                                border.color: "gray"
                                border.width: 1
                            }
                        }

                        Label {
                            text: qsTr("Value color:")
                            Layout.alignment: Qt.AlignRight
                        }

                        Button {
                            id: valueColorButton
                            Layout.fillWidth: true
                            text: dashboard.valueColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.valueColor;
                                mainWindow.currentColorTarget = "valueColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.valueColor
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
                            text: dashboard.titleColor.toString().toUpperCase()
                            onClicked: {
                                colorDialog.selectedColor = dashboard.titleColor;
                                mainWindow.currentColorTarget = "titleColor";
                                colorDialog.open();
                            }

                            background: Rectangle {
                                color: dashboard.titleColor
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
                            text: qsTr("Animation duration: %1ms").arg(dashboard.animationDuration)
                            Layout.fillWidth: true
                        }

                        Slider {
                            id: animationDurationSlider
                            Layout.fillWidth: true
                            from: 0
                            to: 2000
                            value: dashboard.animationDuration
                            onValueChanged: {
                                dashboard.animationDuration = value;
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
                            text: qsTr("Increase +1")
                            Layout.fillWidth: true
                            onClicked: dashboard.increaseValue(1.0)
                        }

                        Button {
                            text: qsTr("Decrease -1")
                            Layout.fillWidth: true
                            onClicked: dashboard.decreaseValue(1.0)
                        }

                        Button {
                            text: qsTr("Reset")
                            Layout.fillWidth: true
                            onClicked: dashboard.reset()
                        }

                        Button {
                            text: qsTr("Animate to 75")
                            Layout.fillWidth: true
                            onClicked: dashboard.setValueAnimated(75.0)
                        }
                    }
                }

                // 快速主题组
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

        Label {
            id: statusLabel
            Layout.fillWidth: true
            text: qsTr("Status: Ready")
            horizontalAlignment: Text.AlignHCenter
            padding: 10
        }
    }

    // === ColorDialog 接受信号（替代 arguments.callee 模式）===
    Connections {
        target: colorDialog

        function onAccepted() {
            if (!colorDialog.selectedColor) {
                return;
            }
            const c = colorDialog.selectedColor;
            switch (mainWindow.currentColorTarget) {
            case "arcColor":
                dashboard.arcColor = c;
                arcColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "scaleColor":
                dashboard.scaleColor = c;
                scaleColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "pointerColor":
                dashboard.pointerColor = c;
                pointerColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "scaleTextColor":
                dashboard.scaleTextColor = c;
                scaleTextColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "backgroundColor":
                dashboard.backgroundColor = c;
                backgroundColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "valueColor":
                dashboard.valueColor = c;
                valueColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            case "titleColor":
                dashboard.titleColor = c;
                titleColorButton.palette.buttonText = getContrastTextColor(c);
                break;
            }
            mainWindow.currentColorTarget = "";
        }
    }

    // === 仪表盘信号连接 ===
    Connections {
        target: dashboard

        function onValueChanged(value: real) {
            console.log("Value changed to:", dashboard.value);
            statusLabel.text = qsTr("Current value: %1").arg(dashboard.value);
        }

        function onValueIncreased(newValue: real) {
            console.log("Value increased to:", newValue);
            statusLabel.text = qsTr("Value increased to: %1").arg(newValue);
            statusLabel.color = "lightgreen";
        }

        function onValueDecreased(newValue: real) {
            console.log("Value decreased to:", newValue);
            statusLabel.text = qsTr("Value decreased to: %1").arg(newValue);
            statusLabel.color = "lightcoral";
        }

        function onValueReset() {
            console.log("Value reset");
            statusLabel.text = qsTr("Value reset to minimum");
            statusLabel.color = "lightyellow";
        }

        function onAnimationStarted(oldValue: real, newValue: real) {
            console.log("Animation started from", oldValue, "to", newValue);
            statusLabel.text = qsTr("Animating: %1 → %2").arg(oldValue).arg(newValue);
            statusLabel.color = "lightblue";
        }

        function onAnimationFinished(value: real) {
            console.log("Animation finished at:", value);
            statusLabel.text = qsTr("Animation finished: %1").arg(value);
            statusLabel.color = "lightgreen";
        }
    }

    // === 初始化 ===
    Component.onCompleted: {
        initializeColorButtons();
        dashboard.setValueAnimated(25);
        statusLabel.text = qsTr("Dashboard initialized");
    }
}
