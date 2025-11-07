import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Fusion

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: qsTr("Clock Quick Example")

    // 颜色对话框
    ColorDialog {
        id: colorDialog
        title: qsTr("Choose color")
    }

    // 保存默认颜色用于重置
    property color defaultBorderColor: "#505050"
    property color defaultBackgroundColor: "#323232"
    property color defaultHourColor: "#f0f0f0"
    property color defaultMinuteColor: "#dcdcdc"
    property color defaultSecondColor: "#ff5050"
    property color defaultTextColor: "#f0f0f0"

    // 当前选择的颜色属性（用于颜色对话框）
    property color selectedColorForDialog

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

                // 初始颜色设置
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
                    columns: 2
                    anchors.fill: parent

                    // 第一列
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
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.borderColor = colorDialog.selectedColor;
                                    borderColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.borderColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
                            colorDialog.open();
                        }

                        background: Rectangle {
                            color: clock.borderColor
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
                        text: clock.backgroundColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.backgroundColor;
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.backgroundColor = colorDialog.selectedColor;
                                    backgroundColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.backgroundColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
                            colorDialog.open();
                        }

                        background: Rectangle {
                            color: clock.backgroundColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Hour hand color:")
                        Layout.alignment: Qt.AlignRight
                    }

                    Button {
                        id: hourColorButton
                        Layout.fillWidth: true
                        text: clock.hourColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.hourColor;
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.hourColor = colorDialog.selectedColor;
                                    hourColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.hourColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
                            colorDialog.open();
                        }

                        background: Rectangle {
                            color: clock.hourColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    // 第二列
                    Label {
                        text: qsTr("Minute hand color:")
                        Layout.alignment: Qt.AlignRight
                    }

                    Button {
                        id: minuteColorButton
                        Layout.fillWidth: true
                        text: clock.minuteColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.minuteColor;
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.minuteColor = colorDialog.selectedColor;
                                    minuteColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.minuteColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
                            colorDialog.open();
                        }

                        background: Rectangle {
                            color: clock.minuteColor
                            border.color: "gray"
                            border.width: 1
                        }
                    }

                    Label {
                        text: qsTr("Second hand color:")
                        Layout.alignment: Qt.AlignRight
                    }

                    Button {
                        id: secondColorButton
                        Layout.fillWidth: true
                        text: clock.secondColor.toString().toUpperCase()
                        onClicked: {
                            colorDialog.selectedColor = clock.secondColor;
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.secondColor = colorDialog.selectedColor;
                                    secondColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.secondColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
                            colorDialog.open();
                        }

                        background: Rectangle {
                            color: clock.secondColor
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
                            colorDialog.accepted.connect(function () {
                                if (colorDialog.selectedColor) {
                                    clock.textColor = colorDialog.selectedColor;
                                    textColorButton.palette.buttonText = mainWindow.getContrastTextColor(clock.textColor);
                                }
                                colorDialog.accepted.disconnect(arguments.callee);
                            });
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

            // 动画设置组
            GroupBox {
                title: qsTr("Animation Settings")
                Layout.fillWidth: true

                ColumnLayout {
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

            // 操作按钮
            GroupBox {
                title: qsTr("Actions")
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent

                    Button {
                        text: qsTr("Update Time")
                        Layout.fillWidth: true
                        onClicked: mainWindow.updateTime()
                    }

                    Button {
                        text: qsTr("Toggle Fullscreen")
                        Layout.fillWidth: true
                        onClicked: mainWindow.visibility === Window.Windowed ? mainWindow.showFullScreen() : mainWindow.showNormal()
                    }
                }
            }

            // 状态信息
            GroupBox {
                title: qsTr("Status")
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent

                    Label {
                        text: qsTr("Smooth mode: ") + (clock.smoothSeconds ? qsTr("Enabled") : qsTr("Disabled"))
                        font.pixelSize: 12
                    }

                    Label {
                        text: qsTr("Seconds shown: ") + (clock.showSeconds ? qsTr("Yes") : qsTr("No"))
                        font.pixelSize: 12
                    }

                    Label {
                        text: qsTr("Last update: ") + mainWindow.formatTime(clock.currentTime)
                        font.pixelSize: 10
                        font.italic: true
                    }
                }
            }
        }
    }

    // === 工具函数 ===

    // 格式化时间显示
    function formatTime(date) {
        return date.toLocaleTimeString(Qt.locale(), "hh:mm:ss");
    }

    // 计算对比度文本颜色
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

    // 更新时间显示
    function updateTime() {
        clock.currentTime = new Date();
        timeLabel.text = qsTr("Current time: ") + formatTime(clock.currentTime);
    }

    // === 主题函数 ===

    function applyClassicTheme() {
        clock.borderColor = "#505050";
        clock.backgroundColor = "#323232";
        clock.hourColor = "#f0f0f0";
        clock.minuteColor = "#dcdcdc";
        clock.secondColor = "#ff5050";
        clock.textColor = "#f0f0f0";
    }

    function applyDarkTheme() {
        clock.borderColor = "white";
        clock.backgroundColor = "darkgray";
        clock.hourColor = "white";
        clock.minuteColor = "lightgray";
        clock.secondColor = "yellow";
        clock.textColor = "white";
    }

    function applyModernTheme() {
        clock.borderColor = "#4682B4";
        clock.backgroundColor = "#F0F8FF";
        clock.hourColor = "#191970";
        clock.minuteColor = "#4169E1";
        clock.secondColor = "#DC143C";
        clock.textColor = "#2F4F4F";
    }

    function resetColors() {
        clock.borderColor = defaultBorderColor;
        clock.backgroundColor = defaultBackgroundColor;
        clock.hourColor = defaultHourColor;
        clock.minuteColor = defaultMinuteColor;
        clock.secondColor = defaultSecondColor;
        clock.textColor = defaultTextColor;
    }

    // === 初始化 ===
    Component.onCompleted: {
        // 初始更新时间显示
        updateTime();

        // 设置定时更新状态
        statusTimer.start();
    }

    // 状态更新定时器
    Timer {
        id: statusTimer
        interval: 1000
        running: true
        repeat: true
    }
}
