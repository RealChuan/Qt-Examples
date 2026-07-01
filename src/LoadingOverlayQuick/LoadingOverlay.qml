import QtQuick
import QtQuick.Controls

Popup {
    id: root

    // === 公共属性 ===
    property string text: ""
    property color textColor: "#1c1c1e"            // iOS label
    property color animationColor: "#007aff"       // iOS systemBlue
    property color cardBackgroundColor: "#F2F2F7F2" // iOS secondarySystemBackground (95% opacity)
    property color dimColor: "#00000066"           // 40% 黑色遮罩
    property int animationSpeed: 100

    // 动画几何参数 (与 LoadingIndicator.qml 同步)
    property int dotCount: 8
    property int dotRadius: 6
    readonly property real _cardWidth: 220
    readonly property real _animSize: 80
    readonly property real _textSpacing: 16

    // === 私有数据 (与 CircularProgressQuick 一致使用 privateData 命名) ===
    QtObject {
        id: privateData
        property int currentFrame: 0
    }

    anchors.centerIn: parent
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    padding: 0
    width: _cardWidth
    background: Rectangle {
        // 暗色遮罩
        color: root.dimColor
    }

    // === 内容卡片 ===
    contentItem: Rectangle {
        id: card
        width: root._cardWidth
        implicitHeight: contentColumn.implicitHeight + 32 * 2
        color: root.cardBackgroundColor
        radius: 14  // iOS 卡片圆角

        Column {
            id: contentColumn
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            spacing: root._textSpacing

            // === RotatingDots 动画 ===
            Item {
                width: root._animSize
                height: root._animSize
                anchors.horizontalCenter: parent.horizontalCenter

                readonly property real _centerX: width / 2
                readonly property real _centerY: height / 2
                readonly property real _radius: Math.min(width, height) * 0.25

                Repeater {
                    model: root.dotCount

                    Rectangle {
                        required property int index

                        width: root.dotRadius * 2
                        height: root.dotRadius * 2
                        radius: root.dotRadius
                        color: root.animationColor

                        readonly property real angle: 2 * Math.PI * index / root.dotCount - 2 * Math.PI * privateData.currentFrame / 60.0
                        readonly property real opacityValue: 0.3 + 0.7 * (Math.sin(angle + privateData.currentFrame * 0.1) + 1) / 2

                        x: parent._centerX + parent._radius * Math.cos(angle) - root.dotRadius
                        y: parent._centerY + parent._radius * Math.sin(angle) - root.dotRadius
                        opacity: opacityValue
                    }
                }
            }

            // === 文本标签 ===
            Text {
                text: root.text
                color: root.textColor
                font.bold: true
                font.pixelSize: 14
                anchors.horizontalCenter: parent.horizontalCenter
                visible: root.text !== ""
                horizontalAlignment: Text.AlignHCenter
            }
        }

        // === 动画定时器 ===
        Timer {
            id: animationTimer
            interval: root.animationSpeed
            running: root.visible
            repeat: true

            onTriggered: {
                privateData.currentFrame = (privateData.currentFrame + 1) % 1000;
            }
        }
    }

    // === 公共方法 ===
    function show(message) {
        if (message !== undefined) {
            text = message;
        }
        open();
    }

    function hide() {
        close();
    }
}
