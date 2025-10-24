#ifndef NORMALCHARTDATA_H
#define NORMALCHARTDATA_H

#include <QList>
#include <QPointF>

using PointList = QList<QPointF>;

auto generateRandomDataPoints(int listCount, int valueMax) -> PointList;

#endif // NORMALCHARTDATA_H
