#ifndef DYNAMICCHARTX_H
#define DYNAMICCHARTX_H

#include "chartview.h"

class DynamicChartXPrivate;
class DynamicChartX : public ChartView
{
public:
    DynamicChartX(QWidget *parent = nullptr);
    ~DynamicChartX() override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void dataReceived(int value);
    void startChart();
    void setupChart();
    DynamicChartXPrivate *d;
};

#endif // DYNAMICCHARTX_H
