#include "piechart.h"
#include "normalchartdata.h"

class PieChartPrivate{
public:
    PieChartPrivate(ChartView *parent)
        :owner(parent){
        pieSeries = new QPieSeries(owner);

        chart = new QChart;
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTitle(QObject::tr("Pie Chart"));
        chart->addSeries(pieSeries);
    }
    ChartView *owner;
    QChart *chart;
    QPieSeries *pieSeries;
};

PieChart::PieChart(QWidget *parent)
    :ChartView(parent)
    ,d(new PieChartPrivate(this))
{
    setupUI();
}

PieChart::~PieChart()
{
    delete d;
}

void PieChart::setupUI()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d->chart);
    PointList pointList = generateRandomDataPoints(5, 101);
    for(int i=0; i<pointList.size(); i++){
        QPieSlice *slice = d->pieSeries->append(tr("P%1").arg(i), pointList[i].y());
        if(i == 0){
            // Show the first slice exploded with label
            slice->setLabelVisible();
            slice->setExploded();
            //slice->setExplodeDistanceFactor(0.5);
        }
    }
}
