pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root
    implicitWidth: 300
    implicitHeight: 50

    // 公共属性
    property alias text: textField.text
    property string placeholderText: qsTr("Enter password")
    property alias maximumLength: textField.maximumLength
    property alias validator: textField.validator
    property bool passwordVisible: false
    property bool capsLockWarningEnabled: true

    // 图标属性
    property string visibleIcon: "👁️"
    property string hiddenIcon: "🔒"

    // 信号
    signal editingFinished

    // 私有状态
    QtObject {
        id: internal

        property bool showCapsLockWarning: false
    }

    // 背景
    Rectangle {
        id: background
        anchors.fill: parent
        color: textField.activeFocus ? "#ffffff" : "#fafafa"
        border.color: textField.activeFocus ? "#0078d4" : "#cccccc"
        border.width: textField.activeFocus ? 2 : 1
        radius: 4
    }

    // 文本输入框
    TextField {
        id: textField
        anchors {
            left: parent.left
            right: toggleButton.left
            top: parent.top
            bottom: parent.bottom
            leftMargin: 8
            rightMargin: 4
        }

        echoMode: root.passwordVisible ? TextInput.Normal : TextInput.Password
        selectByMouse: true
        clip: true
        font.pixelSize: 14
        verticalAlignment: Text.AlignVCenter
        placeholderText: root.placeholderText
        placeholderTextColor: "#999999"
        color: "#1c1c1e"
        background: null

        validator: RegularExpressionValidator {
            regularExpression: /[\x20-\x7E]+/
        }

        onEditingFinished: root.editingFinished()

        Keys.onPressed: event => {
            if (event.key === Qt.Key_CapsLock && root.capsLockWarningEnabled) {
                capsLockCheckTimer.start();
            }
        }

        onActiveFocusChanged: {
            if (activeFocus && root.capsLockWarningEnabled) {
                capsLockCheckTimer.start();
            }
            if (!activeFocus) {
                internal.showCapsLockWarning = false;
            }
        }
    }

    // 显示/隐藏密码按钮
    Rectangle {
        id: toggleButton
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 6
        }
        width: 32
        height: 32
        radius: 4
        color: toggleMouseArea.containsPress ? "#e0e0e0" : toggleMouseArea.containsMouse ? "#f5f5f5" : "transparent"

        Text {
            anchors.centerIn: parent
            text: root.passwordVisible ? root.visibleIcon : root.hiddenIcon
            font.pixelSize: 16
        }

        MouseArea {
            id: toggleMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.togglePasswordVisibility()
        }

        ToolTip {
            text: root.passwordVisible ? qsTr("Hide password") : qsTr("Show password")
            delay: 500
        }
    }

    // CapsLock 警告提示
    Rectangle {
        id: capsLockWarning
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.top
            bottomMargin: 4
        }
        width: capsLockWarningText.implicitWidth + 16
        height: capsLockWarningText.implicitHeight + 8
        radius: 4
        color: "#FFF3CD"
        border.color: "#FFEaa8"
        border.width: 1
        visible: internal.showCapsLockWarning

        Text {
            id: capsLockWarningText
            anchors.centerIn: parent
            text: qsTr("Caps Lock is ON")
            color: "#856404"
            font.pixelSize: 12
        }
    }

    // CapsLock 检测定时器
    Timer {
        id: capsLockCheckTimer
        interval: 50
        onTriggered: {
            // QML 无法直接检测 CapsLock 硬件状态
            // 此处通过 CapsLock 键事件触发，切换警告显示
            if (textField.activeFocus && root.capsLockWarningEnabled) {
                internal.showCapsLockWarning = !internal.showCapsLockWarning;
                if (internal.showCapsLockWarning) {
                    capsLockAutoHideTimer.start();
                }
            }
        }
    }

    // CapsLock 警告自动隐藏定时器
    Timer {
        id: capsLockAutoHideTimer
        interval: 3000
        onTriggered: {
            internal.showCapsLockWarning = false;
        }
    }

    // 公共方法
    function togglePasswordVisibility(): void {
        root.passwordVisible = !root.passwordVisible;
    }

    function setPasswordVisible(visible: bool): void {
        root.passwordVisible = visible;
    }

    function setToggleIcons(visibleIcon: string, hiddenIcon: string): void {
        root.visibleIcon = visibleIcon;
        root.hiddenIcon = hiddenIcon;
    }

    function clear(): void {
        textField.clear();
    }

    function selectAll(): void {
        textField.selectAll();
    }
}
