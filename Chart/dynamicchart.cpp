#include "dynamicchart.h"

class DynamicChartPrivate
{
public:
    DynamicChartPrivate(ChartView *parent)
        : owner(parent)
        , timerId(0)
    {
        splineSeries = new QSplineSeries(owner);
        scatterSeries = new QScatterSeries(owner);
        scatterSeries->setMarkerSize(8);

        chart = new QChart;
        chart->setTitle(QObject::tr("Real Time Dynamic Curve"));
        chart->addSeries(splineSeries);
        chart->addSeries(scatterSeries);
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setRange(0, 100);
        chart->axes(Qt::Vertical).first()->setRange(0, 100);
    }

    ChartView *owner;
    QChart *chart;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    int timerId;
    QList<double> data;
};

DynamicChart::DynamicChart(QWidget *parent)
    : ChartView(parent)
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
        int newData = QRandomGenerator::global()->generate() % (100 + 1);
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
    int dx = 100 / 10;
    int less = 10 - d->data.size();

    for (int i = 0; i < d->data.size(); ++i) {
        d->splineSeries->append(less * dx + i * dx, d->data.at(i));
        d->scatterSeries->append(less * dx + i * dx, d->data.at(i));
    }
}

void DynamicChart::startChart()
{
    d->timerId = startTimer(1000);
    //QRandomGenerator::global()->seed(QDateTime::currentSecsSinceEpoch());
}

void DynamicChart::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
}
