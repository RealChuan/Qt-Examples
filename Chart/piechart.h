#ifndef PIECHART_H
#define PIECHART_H

#include "chartview.h"

class PieChartPrivate;
class PieChart : public ChartView
{
public:
    PieChart(QWidget *parent = nullptr);
    ~PieChart();

private:
    void setupUI();
    PieChartPrivate *d;
};

#endif // PIECHART_H
