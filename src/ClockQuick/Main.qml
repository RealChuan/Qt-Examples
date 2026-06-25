import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 350
    visible: true
    title: qsTr("Clock Quick Example")

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    // 保存默认颜色用于重置 (iOS 调色板)
    readonly property color defaultBorderColor: "#d1d1d6"
    readonly property color defaultBackgroundColor: "#f2f2f7"
    readonly property color defaultHourColor: "#1c1c1e"
    readonly property color defaultMinuteColor: "#3a3a3c"
    readonly property color defaultSecondColor: "#ff3b30"
    readonly property color defaultTextColor: "#1c1c1e"

    // 当前编辑的颜色目标
    property string currentColorTarget: ""

    // 颜色对话框 Connections (替代 arguments.callee)
    Connections {
        target: colorDialog
        function onAccepted() {
            if (!colorDialog.selectedColor)
                return;

            switch (mainWindow.currentColorTarget) {
            case "border":
                clock.borderColor = colorDialog.selectedColor;
                borderColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.borderColor);
                break;
            case "background":
                clock.backgroundColor = colorDialog.selectedColor;
                backgroundColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.backgroundColor);
                break;
            case "hour":
                clock.hourColor = colorDialog.selectedColor;
                hourColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.hourColor);
                break;
            case "minute":
                clock.minuteColor = colorDialog.selectedColor;
                minuteColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.minuteColor);
                break;
            case "second":
                clock.secondColor = colorDialog.selectedColor;
                secondColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.secondColor);
                break;
            case "text":
                clock.textColor = colorDialog.selectedColor;
                textColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.textColor);
                break;
            }
            mainWindow.currentColorTarget = "";
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        // === 左侧：时钟显示区域 ===
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // 时钟组件
            Clock {
                id: clock
                Layout.fillWidth: true
                Layout.fillHeight: true

                borderColor: mainWindow.defaultBorderColor
                backgroundColor: mainWindow.defaultBackgroundColor
                hourColor: mainWindow.defaultHourColor
                minuteColor: mainWindow.defaultMinuteColor
                secondColor: mainWindow.defaultSecondColor
                textColor: mainWindow.defaultTextColor
                smoothSeconds: true
                showSeconds: true
            }

            // 当前时间显示
            Label {
                id: timeLabel
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Current time: ") + mainWindow.formatTime(clock.currentTime)
                font.pixelSize: 14
                font.bold: true
            }
        }

        // === 右侧：控制面板 ===
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // 颜色设置组
            GroupBox {
                title: qsTr("Color Settings")
                Layout.fillWidth: true

                GridLayout {
                    columns: 4
                    anchors.fill: parent

                    Label {
                        text: qsTr("Border color:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Button {
                        id: borderColorButton
                        Layout.fillWidth: true
                        text: clock.borderColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.borderColor;
                            mainWindow.currentColorTarget = "border";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.borderColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Minute hand:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Button {
                        id: minuteColorButton
                        Layout.fillWidth: true
                        text: clock.minuteColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.minuteColor;
                            mainWindow.currentColorTarget = "minute";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.minuteColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Background:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Button {
                        id: backgroundColorButton
                        Layout.fillWidth: true
                        text: clock.backgroundColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.backgroundColor;
                            mainWindow.currentColorTarget = "background";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.backgroundColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Second hand:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Button {
                        id: secondColorButton
                        Layout.fillWidth: true
                        text: clock.secondColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.secondColor;
                            mainWindow.currentColorTarget = "second";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.secondColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Hour hand:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Button {
                        id: hourColorButton
                        Layout.fillWidth: true
                        text: clock.hourColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.hourColor;
                            mainWindow.currentColorTarget = "hour";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.hourColor
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
                        text: clock.textColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.textColor;
                            mainWindow.currentColorTarget = "text";
                            colorDialog.open();
                        }
                        background: Rectangle {
                            color: clock.textColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }
                }
            }

            // 快速主题组
            GroupBox {
                title: qsTr("Quick Themes")
                Layout.fillWidth: true

                RowLayout {
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

            // 动画设置组
            GroupBox {
                title: qsTr("Animation Settings")
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent

                    CheckBox {
                        id: smoothSecondsCheckbox
                        text: qsTr("Smooth seconds")
                        checked: clock.smoothSeconds
                        onToggled: clock.smoothSeconds = checked
                    }

                    CheckBox {
                        id: showSecondsCheckbox
                        text: qsTr("Show seconds")
                        checked: clock.showSeconds
                        onToggled: clock.showSeconds = checked
                    }
                }
            }

            // 操作按钮
            RowLayout {
                Layout.fillWidth: true

                Button {
                    text: qsTr("Update Time")
                    Layout.fillWidth: true
                    onClicked: mainWindow.updateTime()
                }

                Button {
                    text: qsTr("Reset Colors")
                    Layout.fillWidth: true
                    onClicked: mainWindow.resetColors()
                }
            }
        }
    }

    // === 工具函数 ===

    function formatTime(date: date): string {
        return date.toLocaleTimeString(Qt.locale(), "hh:mm:ss");
    }

    function getContrastTextColor(bgColor: color): color {
        function normalize(x: real): real {
            return x <= 0.03928 ? x / 12.92 : Math.pow((x + 0.055) / 1.055, 2.4);
        }

        function getRelativeLuminance(r: real, g: real, b: real): real {
            return 0.2126 * normalize(r) + 0.7152 * normalize(g) + 0.0722 * normalize(b);
        }

        const luminance = getRelativeLuminance(bgColor.r, bgColor.g, bgColor.b);
        return luminance > 0.179 ? "black" : "white";
    }

    function updateTime(): void {
        clock.currentTime = new Date();
        timeLabel.text = qsTr("Current time: ") + formatTime(clock.currentTime);
    }

    // === 主题函数 (iOS 调色板, 与 QWidget 版本一致) ===

    function applyClassicTheme(): void {
        clock.borderColor = "#d1d1d6";
        clock.backgroundColor = "#f2f2f7";
        clock.hourColor = "#1c1c1e";
        clock.minuteColor = "#3a3a3c";
        clock.secondColor = "#ff3b30";
        clock.textColor = "#1c1c1e";
    }

    function applyDarkTheme(): void {
        clock.borderColor = "#3a3a3c";
        clock.backgroundColor = "#1c1c1e";
        clock.hourColor = "#ffffff";
        clock.minuteColor = "#aeaeb2";
        clock.secondColor = "#ff453a";
        clock.textColor = "#ffffff";
    }

    function applyModernTheme(): void {
        clock.borderColor = "#0055a5";
        clock.backgroundColor = "#003366";
        clock.hourColor = "#ffffff";
        clock.minuteColor = "#cccccc";
        clock.secondColor = "#ff9500";
        clock.textColor = "#ffffff";
    }

    function resetColors(): void {
        clock.borderColor = defaultBorderColor;
        clock.backgroundColor = defaultBackgroundColor;
        clock.hourColor = defaultHourColor;
        clock.minuteColor = defaultMinuteColor;
        clock.secondColor = defaultSecondColor;
        clock.textColor = defaultTextColor;
    }

    // === 初始化 ===
    Component.onCompleted: {
        updateTime();
    }
}
