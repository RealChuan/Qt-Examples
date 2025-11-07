import QtQuick
import QtQuick.Shapes

Shape {
    id: root

    property real centerX: 0
    property real centerY: 0
    property real length: 50
    property color color: "white"
    property real angle: 0

    width: 2
    x: centerX
    y: centerY
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
        strokeColor: "transparent"
        fillColor: root.color
        capStyle: ShapePath.RoundCap
        joinStyle: ShapePath.RoundJoin

        PathMove {
            x: -root.width
            y: root.length * 0.1
        }
        PathLine {
            x: 0
            y: -root.length
        }
        PathLine {
            x: root.width
            y: root.length * 0.1
        }
        PathLine {
            x: -root.width
            y: root.length * 0.1
        }
    }

    rotation: root.angle
    transformOrigin: Item.TopLeft
}
