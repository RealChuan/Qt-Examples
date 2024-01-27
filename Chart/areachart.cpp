#include "areachart.h"
#include "normalchartdata.h"

class AreaChart::AreaChartPrivate
{
public:
    explicit AreaChartPrivate(ChartView *q)
        : q_ptr(q)
    {
        areaSeries = new QAreaSeries(q_ptr);
        scatterSeries = new QScatterSeries(q_ptr);
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setTitle(QObject::tr("Area Chart"));
        chart->addSeries(areaSeries);
        chart->addSeries(scatterSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }

    ChartView *q_ptr;

    QChart *chart;
    QAreaSeries *areaSeries;
    QScatterSeries *scatterSeries;
};

AreaChart::AreaChart(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new AreaChartPrivate(this))
{
    setupUI();
}

AreaChart::~AreaChart() = default;

void AreaChart::setupUI()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
    PointList pointList = generateRandomDataPoints(5, 101);
    d_ptr->scatterSeries->replace(pointList);
    auto *upperSeries = new QLineSeries(this);
    auto *lowerSeries = new QLineSeries(this);
    upperSeries->replace(pointList);
    for (auto &i : pointList) {
        i.setY(0);
    }
    lowerSeries->replace(pointList);
    d_ptr->areaSeries->setUpperSeries(upperSeries);
    d_ptr->areaSeries->setLowerSeries(lowerSeries);
}
