#include "dynamicchartx.h"

class DynamicChartXPrivate{
public:
    DynamicChartXPrivate(QChartView *parent)
        : owner(parent)
        , timerId(0)
        , xValue(0)
    {
        valueAxis = new QValueAxis;
        splineSeries = new QSplineSeries;
        scatterSeries = new QScatterSeries;
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
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

    QChartView *owner;
    QChart *chart;
    QValueAxis *valueAxis;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    int timerId;
    QList<double> data;
    qreal xValue;
};

DynamicChartX::DynamicChartX(QWidget *parent)
    : QChartView(parent)
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
        int newData = qrand() % (100 + 1);
        dataReceived(newData);
    }
}

void DynamicChartX::dataReceived(int value)
{
    if(!isVisible())
        return;
    qreal x = d->chart->plotArea().width() / 20;
    d->splineSeries->append(d->xValue*100/20, value);
    d->scatterSeries->append(d->xValue*100/20, value);
    if(d->xValue > 20)
        d->chart->scroll(x, 0);
    d->xValue++;
    if(d->splineSeries->points().size() > 20)
    {
        d->splineSeries->points().removeFirst();
        d->scatterSeries->points().removeFirst();
    }
}

void DynamicChartX::startChart()
{
    d->timerId = startTimer(500);
    qsrand(QDateTime::currentDateTime().toTime_t());
}

void DynamicChartX::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
}
