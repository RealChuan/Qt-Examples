pragma ComponentBehavior: Bound

import QtQuick

// iOS 风格加载指示器，与 LoadingIndicator (QWidget) 版本保持几何与色板一致。
Item {
    id: root

    // === 公共属性 ===
    property string text: ""
    property color textColor: "#1c1c1e"      // iOS label
    property color color: "#007aff"          // iOS systemBlue
    property color backgroundColor: "#F2F2F7E6" // iOS secondarySystemBackground (90% opacity)
    property int animationSpeed: 100
    property int animationStyle: LoadingIndicator.AnimationStyle.RotatingDots

    // 动画样式枚举 (与 C++ AnimationStyle 一致)
    enum AnimationStyle {
        RotatingDots = 0,
        PulsingCircle = 1,
        BouncingBars = 2,
        CustomMovie = 3
    }

    // 动画几何参数 (与 QWidget 版本同步)
    property int dotCount: 8
    property int dotRadius: 6
    property int barCount: 8
    property int barWidth: 6

    // 自定义 GIF 动画
    property url movieSource: ""
    property bool moviePlaying: false

    // === 私有数据 (与 CircularProgressQuick 一致使用 privateData 命名) ===
    QtObject {
        id: privateData
        property int currentFrame: 0
        property real pulseProgress: 0.0
        property int pulseDirection: 1
    }

    // === 几何常量 (与 QWidget 版本同步) ===
    readonly property real _minSize: Math.min(width, height)
    readonly property real _rotatingRadius: _minSize * 0.25
    readonly property real _pulsingMaxRadius: _minSize * 0.3
    readonly property real _barMaxHeight: _minSize * 0.3
    readonly property real _centerX: width / 2
    readonly property real _centerY: height / 2

    // 隐式尺寸
    implicitWidth: 100
    implicitHeight: 100

    // === 背景 ===
    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    // === 旋转点动画 ===
    Item {
        id: rotatingDotsLayer
        anchors.fill: parent
        visible: root.animationStyle === LoadingIndicator.AnimationStyle.RotatingDots

        Repeater {
            model: root.dotCount

            Rectangle {
                required property int index

                width: root.dotRadius * 2
                height: root.dotRadius * 2
                radius: root.dotRadius
                color: root.color

                readonly property real angle: 2 * Math.PI * index / root.dotCount - 2 * Math.PI * privateData.currentFrame / 60.0
                readonly property real opacityValue: 0.3 + 0.7 * (Math.sin(angle + privateData.currentFrame * 0.1) + 1) / 2

                x: root._centerX + root._rotatingRadius * Math.cos(angle) - root.dotRadius
                y: root._centerY + root._rotatingRadius * Math.sin(angle) - root.dotRadius
                opacity: opacityValue
            }
        }
    }

    // === 脉冲圆动画 ===
    Rectangle {
        id: pulseCircle
        anchors.centerIn: parent
        readonly property real _currentRadius: root._pulsingMaxRadius * (0.5 + 0.5 * privateData.pulseProgress)
        width: _currentRadius * 2
        height: _currentRadius * 2
        radius: _currentRadius
        color: "transparent"
        border.color: root.color
        border.width: root.dotRadius
        opacity: 0.7 + 0.3 * privateData.pulseProgress
        visible: root.animationStyle === LoadingIndicator.AnimationStyle.PulsingCircle
    }

    // === 弹跳条动画 ===
    Item {
        id: bouncingBarsLayer
        anchors.fill: parent
        visible: root.animationStyle === LoadingIndicator.AnimationStyle.BouncingBars

        readonly property real _totalWidth: root.barCount * root.barWidth * 2 // 间距 = barWidth
        readonly property real _startX: root._centerX - _totalWidth / 2

        Repeater {
            model: root.barCount

            Rectangle {
                required property int index

                readonly property real _progress: (Math.sin(privateData.currentFrame * 0.2 + index * 0.5) + 1) / 2
                readonly property real _height: root._barMaxHeight * (0.3 + 0.7 * _progress)

                width: root.barWidth
                height: _height
                x: bouncingBarsLayer._startX + index * root.barWidth * 2
                y: root._centerY - _height / 2
                color: root.color
                opacity: 0.3 + 0.7 * _progress
                radius: root.barWidth / 2  // iOS 风格圆角条 (胶囊形)
            }
        }
    }

    // === 自定义 GIF 动画 ===
    AnimatedImage {
        id: movieAnimation
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) / 2
        height: width
        visible: root.animationStyle === LoadingIndicator.AnimationStyle.CustomMovie
        playing: root.visible && root.moviePlaying
        source: root.movieSource
        fillMode: Image.PreserveAspectFit

        // 加载失败时显示备用文本
        Text {
            anchors.centerIn: parent
            text: qsTr("Movie\nNot Found")
            color: root.textColor
            font.pixelSize: 12
            visible: movieAnimation.status === Image.Error
            horizontalAlignment: Text.AlignHCenter
        }
    }

    // === 文本标签 (位于动画下方，间距 20px) ===
    Text {
        id: textLabel
        text: root.text
        color: root.textColor
        font.bold: true
        font.pixelSize: Math.max(12, Math.min(root.width, root.height) / 15)
        // 文本位于动画区域 (centerY + minSize*0.3) 下方 20px
        y: root._centerY + root._minSize * 0.3 + 20
        anchors.horizontalCenter: parent.horizontalCenter
        visible: root.text !== ""
        horizontalAlignment: Text.AlignHCenter
    }

    // === 动画定时器 (不用于 GIF 动画) ===
    Timer {
        id: animationTimer
        interval: root.animationSpeed
        running: root.visible && root.animationStyle !== LoadingIndicator.AnimationStyle.CustomMovie
        repeat: true

        onTriggered: {
            privateData.currentFrame = (privateData.currentFrame + 1) % 1000;

            privateData.pulseProgress += 0.02 * privateData.pulseDirection;
            if (privateData.pulseProgress >= 1.0) {
                privateData.pulseProgress = 1.0;
                privateData.pulseDirection = -1;
            } else if (privateData.pulseProgress <= 0.0) {
                privateData.pulseProgress = 0.0;
                privateData.pulseDirection = 1;
            }
        }
    }

    // === 公共方法 ===

    // 作为覆盖层显示在指定父项上
    function showOverlay(parentItem) {
        if (!parentItem) {
            console.warn("LoadingIndicator: showOverlay requires a parent item");
            return;
        }

        parent = parentItem;
        anchors.fill = parentItem;
        visible = true;
        z = 9999; // 确保在最上层

        if (animationStyle === LoadingIndicator.AnimationStyle.CustomMovie) {
            moviePlaying = true;
        }
    }

    // 隐藏覆盖层
    function hideOverlay() {
        visible = false;
        moviePlaying = false;
        // 注意：不重置 parent，由调用者管理生命周期
    }

    // 设置 GIF 源并切换到 CustomMovie 样式
    function setMovie(source) {
        movieSource = source;
        animationStyle = LoadingIndicator.AnimationStyle.CustomMovie;
    }

    // 动画样式变化处理
    onAnimationStyleChanged: {
        if (animationStyle === LoadingIndicator.AnimationStyle.CustomMovie) {
            animationTimer.running = false;
            moviePlaying = true;
        } else {
            animationTimer.running = true;
            moviePlaying = false;
        }
    }

    // 可见性变化处理
    onVisibleChanged: {
        if (visible && animationStyle === LoadingIndicator.AnimationStyle.CustomMovie) {
            moviePlaying = true;
        } else if (!visible) {
            moviePlaying = false;
        }
    }
}
