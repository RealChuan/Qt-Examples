#ifndef LINECHART_H
#define LINECHART_H

#include "chartview.h"

class LineChartPrivate;
class LineChart : public ChartView
{
    Q_OBJECT
public:
    LineChart(QWidget *parent = nullptr);
    ~LineChart() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void keepToolTip();
    void toolTip(QPointF point, bool state);

    void showDialog();

private:
    void setupUI();
    void buildConnect();
    LineChartPrivate *d;
};

#endif // LINECHART_H
