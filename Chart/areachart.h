#ifndef AREACHART_H
#define AREACHART_H

#include "chartview.h"

class AreaChart : public ChartView
{
public:
    explicit AreaChart(QWidget *parent = nullptr);
    ~AreaChart() override;

private:
    void setupUI();

    class AreaChartPrivate;
    QScopedPointer<AreaChartPrivate> d_ptr;
};

#endif // AREACHART_H
