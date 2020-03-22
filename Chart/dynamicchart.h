#ifndef DYNAMICCHART_H
#define DYNAMICCHART_H

#include "chartview.h"

class DynamicChartPrivate;
class DynamicChart : public ChartView
{
public:
    DynamicChart(QWidget *parent = nullptr);
    ~DynamicChart() override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void dataReceived(int value);
    void startChart();
    void setupChart();
    DynamicChartPrivate *d;
};

#endif // DYNAMICCHART_H
