#ifndef STACKEDBARCHART_H
#define STACKEDBARCHART_H

#include "chartview.h"

class StackedBarChart : public ChartView
{
public:
    explicit StackedBarChart(QWidget *parent = nullptr);
    ~StackedBarChart() override;

private:
    void setupUI();

    class StackedBarChartPrivate;
    QScopedPointer<StackedBarChartPrivate> d_ptr;
};

#endif // STACKEDBARCHART_H
