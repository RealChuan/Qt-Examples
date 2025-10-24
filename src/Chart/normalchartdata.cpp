#include "normalchartdata.h"

#include <QRandomGenerator>

auto generateRandomDataPoints(int listCount, int valueMax) -> PointList
{
    PointList pointList;
    qreal yValue(0);
    int count = valueMax / listCount;
    for (int i = 0; i < valueMax; i += count) {
        yValue = yValue + QRandomGenerator::global()->bounded(valueMax / qreal(listCount));
        QPointF pointF(i, yValue);
        pointList.append(pointF);
    }
    return pointList;
}
