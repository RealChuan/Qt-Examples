#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QtCharts>

class ChartView : public QChartView
{
public:
    explicit ChartView(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // CHARTVIEW_H
