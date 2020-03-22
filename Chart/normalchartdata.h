#ifndef NORMALCHARTDATA_H
#define NORMALCHARTDATA_H

#include <QPointF>
#include <QList>
#include <QRandomGenerator>

typedef QList<QPointF> PointList;

PointList generateRandomDataPoints(int listCount, int valueMax);

#endif // NORMALCHARTDATA_H
