#include "stackedbarchart.h"
#include "normalchartdata.h"

class StackedBarChart::StackedBarChartPrivate
{
public:
    explicit StackedBarChartPrivate(ChartView *parent)
        : q_ptr(parent)
    {
        stackedBarSeries = new QStackedBarSeries(q_ptr);
        PointList pointList = generateRandomDataPoints(5, 101);
        QBarSet *set = new QBarSet("Bar set", q_ptr);
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

    ChartView *q_ptr;

    QChart *chart;
    QStackedBarSeries *stackedBarSeries;
};

StackedBarChart::StackedBarChart(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new StackedBarChartPrivate(this))
{
    setupUI();
}

StackedBarChart::~StackedBarChart() = default;

void StackedBarChart::setupUI()
{
    setChart(d_ptr->chart);
}
