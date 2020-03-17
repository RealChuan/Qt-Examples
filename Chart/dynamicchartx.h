#ifndef DYNAMICCHARTX_H
#define DYNAMICCHARTX_H

#include <QtCharts>

class DynamicChartXPrivate;
class DynamicChartX : public QChartView
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
