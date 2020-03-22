#include "areachart.h"
#include "normalchartdata.h"

class AreaChartPrivate{
public:
    AreaChartPrivate(ChartView *parent)
        :owner(parent){
        areaSeries = new QAreaSeries(owner);
        scatterSeries = new QScatterSeries(owner);
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setTitle(QObject::tr("Area Chart"));
        chart->addSeries(areaSeries);
        chart->addSeries(scatterSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }
    ChartView *owner;
    QChart *chart;
    QAreaSeries *areaSeries;
    QScatterSeries *scatterSeries;
};

AreaChart::AreaChart(QWidget *parent)
    :ChartView(parent)
    ,d(new AreaChartPrivate(this))
{
    setupUI();
}

AreaChart::~AreaChart()
{
    delete d;
}

void AreaChart::setupUI()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
    PointList pointList = generateRandomDataPoints(5, 101);
    d->scatterSeries->replace(pointList);
    QLineSeries *upperSeries = new QLineSeries(this);
    QLineSeries *lowerSeries = new QLineSeries(this);
    upperSeries->replace(pointList);
    for(int i=0; i<pointList.size(); i++){
        pointList[i].setY(0);
    }
    lowerSeries->replace(pointList);
    d->areaSeries->setUpperSeries(upperSeries);
    d->areaSeries->setLowerSeries(lowerSeries);
}
