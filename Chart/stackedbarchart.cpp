#include "stackedbarchart.h"
#include "normalchartdata.h"

class StackedBarChartPrivate{
public:
    StackedBarChartPrivate(ChartView *parent)
        :owner(parent){
        stackedBarSeries = new QStackedBarSeries(owner);
        PointList pointList = generateRandomDataPoints(5, 101);
        QBarSet *set = new QBarSet("Bar set", owner);
        foreach(const QPointF& p, pointList)
            *set << p.y();
        stackedBarSeries->append(set);  //  先添加点集，不然x轴为空

        chart = new QChart;
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTitle(QObject::tr("StackedBar Chart"));
        chart->addSeries(stackedBarSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }
    ChartView *owner;
    QChart *chart;
    QStackedBarSeries *stackedBarSeries;
};

StackedBarChart::StackedBarChart(QWidget *parent)
    :ChartView(parent)
    ,d(new StackedBarChartPrivate(this))
{
    setupUI();
}

StackedBarChart::~StackedBarChart()
{
    delete d;
}

void StackedBarChart::setupUI()
{
    setChart(d->chart);
}
