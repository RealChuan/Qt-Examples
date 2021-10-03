#include "dynamicchartx.h"

class DynamicChartXPrivate{
public:
    DynamicChartXPrivate(ChartView *parent)
        : owner(parent)
        , timerId(0)
        , xValue(0)
    {
        valueAxis = new QValueAxis(owner);
        splineSeries = new QSplineSeries(owner);
        scatterSeries = new QScatterSeries(owner);
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTitle(QObject::tr("Real Time Dynamic Curve X"));
        chart->addSeries(splineSeries);
        chart->addSeries(scatterSeries);
        chart->addAxis(valueAxis, Qt::AlignBottom);
        splineSeries->attachAxis(valueAxis);
        scatterSeries->attachAxis(valueAxis);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }

    ChartView *owner;
    QChart *chart;
    QValueAxis *valueAxis;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    int timerId;
    QList<double> data;
    qreal xValue;
};

DynamicChartX::DynamicChartX(QWidget *parent)
    : ChartView(parent)
    , d(new DynamicChartXPrivate(this))
{
    setupChart();
    startChart();
}

DynamicChartX::~DynamicChartX()
{
    delete d;
}

void DynamicChartX::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->timerId) {
        int newData = QRandomGenerator::global()->generate() % (100 + 1);
        dataReceived(newData);
    }
}

void DynamicChartX::dataReceived(int value)
{
    if(!isVisible())
        return;
    qreal x = d->chart->plotArea().width() / 10;
    d->splineSeries->append(d->xValue*100/10, value);
    d->scatterSeries->append(d->xValue*100/10, value);
    if(d->xValue > 10)
        d->chart->scroll(x, 0);
    d->xValue++;
    if(d->splineSeries->points().size() > 10)
    {
        d->splineSeries->points().removeFirst();
        d->scatterSeries->points().removeFirst();
    }
}

void DynamicChartX::startChart()
{
    d->timerId = startTimer(1000);
    //QRandomGenerator::global()->seed(QDateTime::currentSecsSinceEpoch());
}

void DynamicChartX::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
}
