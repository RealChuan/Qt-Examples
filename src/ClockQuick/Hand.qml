import QtQuick
import QtQuick.Shapes

Shape {
    id: root

    property real centerX: 0
    property real centerY: 0
    property real length: 50
    property real handWidth: 2
    property color color: "white"
    property real angle: 0
    property real tailRatio: 0.1

    x: centerX
    y: centerY
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
        strokeColor: "transparent"
        fillColor: root.color
        capStyle: ShapePath.RoundCap
        joinStyle: ShapePath.RoundJoin

        // 菱形指针: 尾端 → 左中 → 尖端 → 右中 (匹配 QWidget 版本)
        PathMove {
            x: 0
            y: root.length * root.tailRatio
        }
        PathLine {
            x: -root.handWidth
            y: 0
        }
        PathLine {
            x: 0
            y: -root.length
        }
        PathLine {
            x: root.handWidth
            y: 0
        }
        PathLine {
            x: 0
            y: root.length * root.tailRatio
        }
    }

    rotation: root.angle
    transformOrigin: Item.TopLeft
}
