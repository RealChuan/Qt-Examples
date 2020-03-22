#ifndef AREACHART_H
#define AREACHART_H

#include "chartview.h"

class AreaChartPrivate;
class AreaChart : public ChartView
{
public:
    AreaChart(QWidget *parent = nullptr);
    ~AreaChart();

private:
    void setupUI();
    AreaChartPrivate *d;
};

#endif // AREACHART_H
