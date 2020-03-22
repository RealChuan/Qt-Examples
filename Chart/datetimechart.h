#ifndef DATETIMECHART_H
#define DATETIMECHART_H

#include "chartview.h"

class DateTimeChartPrivate;
class DateTimeChart : public ChartView
{
public:
    DateTimeChart(QWidget *parent = nullptr);
    ~DateTimeChart();

private:
    void setupUI();
    DateTimeChartPrivate *d;
};

#endif // DATETIMECHART_H
