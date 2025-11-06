pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root

    // Properties
    property list<string> messageList: ["Step 1", "Step 2", "Step 3", "Step 4", "Step 5"]
    property int step: 0
    readonly property int maxStep: messageList.length
    property color backgroundColor: "#505050"
    property color currentBackgroundColor: "#4da1ff"
    property color foregroundColor: "#fefefe"
    property font textFont: Qt.font({
        pixelSize: 14
    })
    property font dateFont: Qt.font({
        pixelSize: 12
    })
    property int spacing: 10
    property int circleRadius: Math.min(width / (maxStep * 5), height / 5)

    // Internal properties
    property list<string> dateList: []

    signal progressCompleted

    // Size hints
    implicitWidth: 400
    implicitHeight: 120

    onMessageListChanged: {
        if (step > maxStep) {
            step = maxStep;
            dateList = dateList.slice(0, step);
        }
        rebuildDateList();
    }

    onStepChanged: {
        if (step === maxStep) {
            progressCompleted();
        }
    }

    Component.onCompleted: {
        rebuildDateList();
    }

    // Public methods
    function dateAt(stepIndex) {
        return stepIndex >= 0 && stepIndex < dateList.length ? dateList[stepIndex] : "";
    }

    function setDateAt(stepIndex, date) {
        if (stepIndex >= 0 && stepIndex < dateList.length) {
            dateList[stepIndex] = date;
        }
    }

    function reset() {
        step = 0;
        dateList = [];
    }

    function next() {
        if (step < maxStep) {
            setStep(step + 1);
        }
    }

    function previous() {
        if (step > 0) {
            setStep(step - 1);
        }
    }

    function setStep(newStep) {
        if (newStep < 0 || newStep > maxStep) {
            console.warn("NavigationProgressBar: Step out of range:", newStep);
            return;
        }

        if (newStep === step)
            return;

        // 记录新步骤的日期（如果是前进）
        if (newStep > step) {
            for (let i = step; i < newStep; i++) {
                dateList.push(new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm:ss"));
            }
        } else {
            if (dateList.length > newStep) {
                dateList = dateList.slice(0, newStep);
            }
        }

        step = newStep;
    }

    // Private methods
    function rebuildDateList() {
        const newDateList = [];
        for (let i = 0; i < step; i++) {
            newDateList.push(new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm:ss"));
        }
        dateList = newDateList;
    }

    // Visual implementation - 使用 Row + Column 确保对齐
    Row {
        id: mainRow
        anchors.fill: parent
        spacing: 0

        Repeater {
            model: root.maxStep

            // 每个步骤的容器
            Item {
                id: stepContainer
                required property int index

                width: mainRow.width / root.maxStep
                height: mainRow.height

                // 每个步骤的垂直布局
                Column {
                    anchors.fill: parent
                    spacing: 5

                    // 步骤文本
                    Label {
                        id: stepLabel
                        width: parent.width
                        height: parent.height / 3
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: root.messageList[stepContainer.index]
                        font: root.textFont
                        color: stepContainer.index < root.step ? root.currentBackgroundColor : root.backgroundColor
                        elide: Text.ElideRight
                    }

                    // 进度线和圆圈区域
                    Item {
                        id: progressArea
                        width: parent.width
                        height: parent.height / 3

                        // 连接线背景（整个宽度）
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            height: root.circleRadius / 3
                            color: root.backgroundColor
                        }

                        // 已完成的连接线
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: stepContainer.index < root.step ? parent.width : 0
                            height: root.circleRadius / 3
                            color: root.currentBackgroundColor
                            visible: stepContainer.index >= 0
                        }

                        // 当前步骤的部分连接线（如果未完成）
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: stepContainer.index === root.step - 1 && root.step < root.maxStep ? parent.width / 2 : 0
                            height: root.circleRadius / 3
                            color: root.currentBackgroundColor
                            visible: stepContainer.index === root.step - 1 && root.step < root.maxStep
                        }

                        // 圆圈和数字
                        Rectangle {
                            id: circle
                            width: root.circleRadius * 2
                            height: root.circleRadius * 2
                            radius: root.circleRadius
                            anchors.centerIn: parent
                            color: stepContainer.index < root.step ? root.currentBackgroundColor : root.backgroundColor

                            Text {
                                anchors.centerIn: parent
                                text: stepContainer.index + 1
                                font: Qt.font({
                                    family: root.textFont.family,
                                    pixelSize: root.circleRadius * 0.8
                                })
                                color: root.foregroundColor
                            }
                        }
                    }

                    // 日期显示
                    Label {
                        id: dateLabel
                        width: parent.width
                        height: parent.height / 3
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font: root.dateFont
                        color: root.currentBackgroundColor
                        elide: Text.ElideRight

                        text: root.dateAt(stepContainer.index)
                    }
                }
            }
        }
    }
}
