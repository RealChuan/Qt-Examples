#ifndef PIECHART_H
#define PIECHART_H

#include "chartview.h"

class PieChart : public ChartView
{
public:
    explicit PieChart(QWidget *parent = nullptr);
    ~PieChart() override;

private:
    void setupUI();

    class PieChartPrivate;
    QScopedPointer<PieChartPrivate> d_ptr;
};

#endif // PIECHART_H
