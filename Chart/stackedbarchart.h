#ifndef STACKEDBARCHART_H
#define STACKEDBARCHART_H

#include "chartview.h"

class StackedBarChartPrivate;
class StackedBarChart : public ChartView
{
public:
    StackedBarChart(QWidget *parent = nullptr);
    ~StackedBarChart();

private:
    void setupUI();
    StackedBarChartPrivate *d;
};

#endif // STACKEDBARCHART_H
