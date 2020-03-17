#include "dynamicchart.h"

class DynamicChartPrivate{
public:
    DynamicChartPrivate(QChartView *parent)
        : owner(parent)
        , timerId(0)
    {
        splineSeries = new QSplineSeries;
        scatterSeries = new QScatterSeries;
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setTitle(QObject::tr("Real Time Dynamic Curve"));
        chart->addSeries(splineSeries);
        chart->addSeries(scatterSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }

    QChartView *owner;
    QChart *chart;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    int timerId;
    QList<double> data;
};

DynamicChart::DynamicChart(QWidget *parent)
    : QChartView(parent)
    , d(new DynamicChartPrivate(this))
{
    setupChart();
    startChart();
}

DynamicChart::~DynamicChart()
{
    delete d;
}

void DynamicChart::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->timerId) {
        int newData = qrand() % (100 + 1);
        dataReceived(newData);
    }
}

void DynamicChart::dataReceived(int value)
{
    d->data << value;

    while (d->data.size() > 30) {
        d->data.removeFirst();
    }

    if (!isVisible())
        return;

    d->splineSeries->clear();
    d->scatterSeries->clear();
    int dx = 100 / 20;
    int less = 20 - d->data.size();

    for (int i = 0; i < d->data.size(); ++i) {
        d->splineSeries->append(less*dx+i*dx, d->data.at(i));
        d->scatterSeries->append(less*dx+i*dx, d->data.at(i));
    }
}

void DynamicChart::startChart()
{
    d->timerId = startTimer(500);
    qsrand(QDateTime::currentDateTime().toTime_t());
}

void DynamicChart::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
}
