import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 300
    height: 50
    border.color: textInput.activeFocus ? "#0078d4" : "#cccccc"
    border.width: textInput.activeFocus ? 2 : 1

    // 公共属性
    property alias text: textInput.text
    property string placeholderText: qsTr("Enter password")
    property alias maximumLength: textInput.maximumLength
    property alias validator: textInput.validator
    property bool passwordVisible: false

    // 图标属性 - 可以使用字体图标或图片路径
    property string visibleIcon: "👁️"  // Show password icon
    property string hiddenIcon: "🔒"   // Hide password icon

    // 信号
    signal editingFinished

    // 输入框容器
    Item {
        id: textInputContainer
        anchors {
            left: parent.left
            right: toggleButton.left
            verticalCenter: parent.verticalCenter
            margins: 12
        }
        height: parent.height

        // 占位符文本
        Text {
            id: placeholder
            anchors.fill: parent
            text: root.placeholderText
            color: "#999999"
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            visible: textInput.text.length === 0 && !textInput.activeFocus
        }

        // 文本输入框
        TextInput {
            id: textInput
            anchors.fill: parent
            echoMode: root.passwordVisible ? TextInput.Normal : TextInput.Password
            selectByMouse: true
            clip: true
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter

            // 密码验证 - 只允许可打印ASCII字符
            validator: RegularExpressionValidator {
                regularExpression: /[\x20-\x7E]+/
            }

            onEditingFinished: root.editingFinished()

            // 当获得焦点时隐藏占位符
            onActiveFocusChanged: {
                if (activeFocus) {
                    placeholder.visible = false;
                } else if (text.length === 0) {
                    placeholder.visible = true;
                }
            }

            // 当文本变化时更新占位符可见性
            onTextChanged: {
                if (text.length > 0) {
                    placeholder.visible = false;
                } else if (!activeFocus) {
                    placeholder.visible = true;
                }
            }
        }
    }

    // 显示/隐藏密码按钮
    Rectangle {
        id: toggleButton
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 8
        }
        width: 32
        height: 32
        radius: 4
        color: mouseArea.containsPress ? "#e0e0e0" : mouseArea.containsMouse ? "#f5f5f5" : "transparent"

        // 图标显示
        Text {
            id: iconText
            anchors.centerIn: parent
            text: root.passwordVisible ? root.visibleIcon : root.hiddenIcon
            font.pixelSize: 16
        }

        // 点击区域
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.togglePasswordVisibility();
            }
        }

        // 按钮提示
        ToolTip {
            id: buttonToolTip
            text: root.passwordVisible ? qsTr("Hide password") : qsTr("Show password")
            delay: 500
        }
    }

    // 方法
    function togglePasswordVisibility() {
        root.passwordVisible = !root.passwordVisible;
    }

    function setToggleIcons(visibleIcon, hiddenIcon) {
        root.visibleIcon = visibleIcon;
        root.hiddenIcon = hiddenIcon;
    }

    function clear() {
        textInput.clear();
        placeholder.visible = true;
    }

    function selectAll() {
        textInput.selectAll();
    }

    // 组件加载完成后的初始化
    Component.onCompleted: {
        // 设置初始状态
        root.passwordVisible = false;
    }
}
