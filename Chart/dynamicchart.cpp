#include "dynamicchart.h"

class DynamicChart::DynamicChartPrivate
{
public:
    explicit DynamicChartPrivate(ChartView *parent)
        : owner(parent)
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
    int timerId = 0;
    QList<double> data;
};

DynamicChart::DynamicChart(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new DynamicChartPrivate(this))
{
    setupChart();
    startChart();
}

DynamicChart::~DynamicChart() = default;

void DynamicChart::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d_ptr->timerId) {
        int newData = QRandomGenerator::global()->generate() % (100 + 1);
        dataReceived(newData);
    }
}

void DynamicChart::dataReceived(int value)
{
    d_ptr->data << value;

    while (d_ptr->data.size() > 30) {
        d_ptr->data.removeFirst();
    }

    if (!isVisible()) {
        return;
    }

    d_ptr->splineSeries->clear();
    d_ptr->scatterSeries->clear();
    int dx = 100 / 10;
    int less = 10 - d_ptr->data.size();

    for (int i = 0; i < d_ptr->data.size(); ++i) {
        d_ptr->splineSeries->append(less * dx + i * dx, d_ptr->data.at(i));
        d_ptr->scatterSeries->append(less * dx + i * dx, d_ptr->data.at(i));
    }
}

void DynamicChart::startChart()
{
    d_ptr->timerId = startTimer(1000);
    //QRandomGenerator::global()->seed(QDateTime::currentSecsSinceEpoch());
}

void DynamicChart::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
}
