#ifndef DYNAMICCHART_H
#define DYNAMICCHART_H

#include "chartview.h"

class DynamicChart : public ChartView
{
public:
    explicit DynamicChart(QWidget *parent = nullptr);
    ~DynamicChart() override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void dataReceived(int value);
    void startChart();
    void setupChart();

    class DynamicChartPrivate;
    QScopedPointer<DynamicChartPrivate> d_ptr;
};

#endif // DYNAMICCHART_H
