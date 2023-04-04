#include "dynamicchartx.h"

class DynamicChartX::DynamicChartXPrivate
{
public:
    explicit DynamicChartXPrivate(ChartView *parent)
        : q_ptr(parent)
    {
        valueAxis = new QValueAxis(q_ptr);
        splineSeries = new QSplineSeries(q_ptr);
        scatterSeries = new QScatterSeries(q_ptr);
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

    ChartView *q_ptr;

    QChart *chart;
    QValueAxis *valueAxis;
    QSplineSeries *splineSeries;
    QScatterSeries *scatterSeries;
    int timerId = 0;
    QList<double> data;
    qreal xValue = 0;
};

DynamicChartX::DynamicChartX(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new DynamicChartXPrivate(this))
{
    setupChart();
    startChart();
}

DynamicChartX::~DynamicChartX() = default;

void DynamicChartX::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d_ptr->timerId) {
        int newData = QRandomGenerator::global()->generate() % (100 + 1);
        dataReceived(newData);
    }
}

void DynamicChartX::dataReceived(int value)
{
    if (!isVisible()) {
        return;
    }
    qreal x = d_ptr->chart->plotArea().width() / 10;
    d_ptr->splineSeries->append(d_ptr->xValue * 100 / 10, value);
    d_ptr->scatterSeries->append(d_ptr->xValue * 100 / 10, value);
    if (d_ptr->xValue > 10) {
        d_ptr->chart->scroll(x, 0);
    }
    d_ptr->xValue++;
    if (d_ptr->splineSeries->points().size() > 10) {
        d_ptr->splineSeries->points().removeFirst();
        d_ptr->scatterSeries->points().removeFirst();
    }
}

void DynamicChartX::startChart()
{
    d_ptr->timerId = startTimer(1000);
    //QRandomGenerator::global()->seed(QDateTime::currentSecsSinceEpoch());
}

void DynamicChartX::setupChart()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
}
