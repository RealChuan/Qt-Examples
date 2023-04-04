#ifndef DATETIMECHART_H
#define DATETIMECHART_H

#include "chartview.h"

class DateTimeChart : public ChartView
{
public:
    explicit DateTimeChart(QWidget *parent = nullptr);
    ~DateTimeChart() override;

private:
    void setupUI();

    class DateTimeChartPrivate;
    QScopedPointer<DateTimeChartPrivate> d_ptr;
};

#endif // DATETIMECHART_H
