#include "datetimechart.h"
#include "normalchartdata.h"

class DateTimeChart::DateTimeChartPrivate
{
public:
    DateTimeChartPrivate(ChartView *parent)
        : owner(parent)
    {
        dateTimeAxisX = new QDateTimeAxis(owner);
        dateTimeAxisX->setFormat("yyyy-MM");
        //dateTimeAxisX->setTitleText(QObject::tr("Date"));

        valueAxisY = new QValueAxis(owner);
        //valueAxisY->setTitleText(QObject::tr("Value"));
        valueAxisY->setRange(0, 100);

        lineSeries = new QLineSeries(owner);
        scatterSeries = new QScatterSeries(owner);
        scatterSeries->setMarkerSize(8);

        PointList pointList = generateRandomDataPoints(5, 101);
        int year = 2020;
        int mouth = 1;
        for (const QPointF &p : qAsConst(pointList)) {
            QDateTime momentInTime(QDate(year, mouth, 15), QTime());
            scatterSeries->append(momentInTime.toMSecsSinceEpoch(), p.y());
            lineSeries->append(momentInTime.toMSecsSinceEpoch(), p.y());
            year++;
            mouth++;
        }

        chart = new QChart;
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTitle(QObject::tr("DateTime Chart"));
        chart->addSeries(lineSeries);
        chart->addSeries(scatterSeries);
        chart->addAxis(dateTimeAxisX, Qt::AlignBottom);
        chart->addAxis(valueAxisY, Qt::AlignLeft);
        lineSeries->attachAxis(dateTimeAxisX);
        lineSeries->attachAxis(valueAxisY);
        scatterSeries->attachAxis(dateTimeAxisX);
        scatterSeries->attachAxis(valueAxisY);
    }
    ChartView *owner;
    QChart *chart;
    QDateTimeAxis *dateTimeAxisX;
    QValueAxis *valueAxisY;
    QLineSeries *lineSeries;
    QScatterSeries *scatterSeries;
};

DateTimeChart::DateTimeChart(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new DateTimeChartPrivate(this))
{
    setupUI();
}

DateTimeChart::~DateTimeChart()
{
}

void DateTimeChart::setupUI()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
}
