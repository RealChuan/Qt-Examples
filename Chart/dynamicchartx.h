#ifndef DYNAMICCHARTX_H
#define DYNAMICCHARTX_H

#include "chartview.h"

class DynamicChartX : public ChartView
{
public:
    explicit DynamicChartX(QWidget *parent = nullptr);
    ~DynamicChartX() override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void dataReceived(int value);
    void startChart();
    void setupChart();

    class DynamicChartXPrivate;
    QScopedPointer<DynamicChartXPrivate> d_ptr;
};

#endif // DYNAMICCHARTX_H
