pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    // 公共属性 - QML会自动为这些属性生成相应的on<Property>Changed信号
    property string text: ""
    property color textColor: "#04B5C8"
    property color color: "#4682E6"
    property color backgroundColor: "#64FFFFFF"
    property int animationSpeed: 100
    property int animationStyle: LoadingIndicator.RotatingDots

    // 动画样式枚举
    enum AnimationStyle {
        RotatingDots = 0,
        PulsingCircle = 1,
        BouncingBars = 2,
        CustomMovie = 3
    }

    // 特定动画参数
    property int dotCount: 8
    property int dotRadius: 6
    property int barCount: 8
    property int barWidth: 6

    // Movie 动画相关属性
    property url movieSource: ""
    property bool moviePlaying: false

    // 内部状态属性
    property int _currentFrame: 0
    property double _pulseProgress: 0.0
    property int _pulseDirection: 1

    // 尺寸策略
    implicitWidth: 100
    implicitHeight: 100

    // 背景
    Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    // 主要内容区域
    Column {
        anchors.centerIn: parent
        spacing: root.text ? 20 : 0

        // 动画显示区域
        Item {
            id: animationContainer
            width: Math.min(parent.parent.width, parent.parent.height) * 0.5
            height: width
            anchors.horizontalCenter: parent.horizontalCenter

            // 旋转点动画
            Repeater {
                model: root.dotCount
                visible: root.animationStyle === LoadingIndicator.RotatingDots

                Rectangle {
                    required property int index

                    width: root.dotRadius * 2
                    height: root.dotRadius * 2
                    radius: root.dotRadius
                    color: root.color
                    visible: root.animationStyle === LoadingIndicator.RotatingDots

                    readonly property real angle: 2 * Math.PI * index / root.dotCount - 2 * Math.PI * root._currentFrame / 60.0
                    readonly property real opacityValue: 0.3 + 0.7 * (Math.sin(angle + root._currentFrame * 0.1) + 1) / 2

                    x: animationContainer.width / 2 + (animationContainer.width / 3) * Math.cos(angle) - root.dotRadius
                    y: animationContainer.height / 2 + (animationContainer.height / 3) * Math.sin(angle) - root.dotRadius
                    opacity: opacityValue
                }
            }

            // 脉冲圆动画
            Rectangle {
                id: pulseCircle
                anchors.centerIn: parent
                width: animationContainer.width * (0.4 + 0.4 * root._pulseProgress)
                height: width
                radius: width / 2
                color: "transparent"
                border.color: root.color
                border.width: root.dotRadius
                opacity: 0.6 + 0.4 * root._pulseProgress
                visible: root.animationStyle === LoadingIndicator.PulsingCircle
            }

            // 弹跳条动画
            Row {
                anchors.centerIn: parent
                spacing: root.barWidth
                visible: root.animationStyle === LoadingIndicator.BouncingBars

                Repeater {
                    model: root.barCount

                    Rectangle {
                        required property int index

                        width: root.barWidth
                        height: animationContainer.height * (0.3 + 0.7 * barProgress)
                        color: root.color
                        opacity: 0.3 + 0.7 * barProgress

                        readonly property real barProgress: (Math.sin(root._currentFrame * 0.2 + index * 0.5) + 1) / 2
                    }
                }
            }

            // Movie 动画
            AnimatedImage {
                id: movieAnimation
                anchors.centerIn: parent
                width: Math.min(animationContainer.width, animationContainer.height)
                height: width
                visible: root.animationStyle === LoadingIndicator.CustomMovie
                playing: root.visible && root.moviePlaying
                source: root.movieSource
                fillMode: Image.PreserveAspectFit

                // 如果加载失败，显示备用文本
                Text {
                    anchors.centerIn: parent
                    text: qsTr("Movie\nNot Found")
                    color: root.textColor
                    font.pixelSize: 12
                    visible: movieAnimation.status === Image.Error
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // 文本标签
        Label {
            id: textLabel
            text: root.text
            color: root.textColor
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
            visible: text !== ""

            // 自动调整字体大小
            font.pixelSize: Math.max(12, Math.min(16, root.height / 15))
        }
    }

    // 动画定时器（不用于 Movie 动画）
    Timer {
        id: animationTimer
        interval: root.animationSpeed
        running: root.visible && root.animationStyle !== LoadingIndicator.CustomMovie
        repeat: true

        onTriggered: {
            root._currentFrame = (root._currentFrame + 1) % 1000;

            // 更新脉冲动画
            root._pulseProgress += 0.02 * root._pulseDirection;
            if (root._pulseProgress >= 1.0) {
                root._pulseProgress = 1.0;
                root._pulseDirection = -1;
            } else if (root._pulseProgress <= 0.0) {
                root._pulseProgress = 0.0;
                root._pulseDirection = 1;
            }
        }
    }

    // 覆盖层显示方法
    function showOverlay(parentItem) {
        if (!parentItem) {
            console.warn("LoadingIndicator: showOverlay requires a parent item");
            return;
        }

        // 设置父级并填充
        parent = parentItem;
        anchors.fill = parentItem;
        visible = true;
        z = 9999; // 确保在最上层

        // 如果是 Movie 动画，开始播放
        if (root.animationStyle === LoadingIndicator.CustomMovie) {
            root.moviePlaying = true;
        }
    }

    // 隐藏覆盖层方法
    function hideOverlay() {
        visible = false;
        // 停止 Movie 动画
        root.moviePlaying = false;
        // 注意：这里不重置parent，让调用者管理生命周期
    }

    // 设置 Movie 源
    function setMovie(source) {
        root.movieSource = source;
        animationStyle = LoadingIndicator.CustomMovie;
    }

    // 设置 Movie 源（字符串版本）
    function setMovieFromString(sourceString) {
        setMovie(sourceString);
    }

    // 组件加载完成后的初始化
    Component.onCompleted: {
        // 确保在组件完成时更新动画速度
        animationTimer.interval = Qt.binding(function () {
            return root.animationSpeed;
        });
    }

    // 监听动画样式变化
    onAnimationStyleChanged: {
        // 当切换到 Movie 动画时，确保停止定时器动画
        if (animationStyle === LoadingIndicator.CustomMovie) {
            animationTimer.running = false;
            root.moviePlaying = true;
        } else {
            animationTimer.running = true;
            root.moviePlaying = false;
        }
    }

    // 监听可见性变化
    onVisibleChanged: {
        if (visible && animationStyle === LoadingIndicator.CustomMovie) {
            root.moviePlaying = true;
        } else if (!visible) {
            root.moviePlaying = false;
        }
    }
}
