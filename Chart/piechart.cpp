#include "piechart.h"
#include "normalchartdata.h"

class PieChart::PieChartPrivate
{
public:
    explicit PieChartPrivate(ChartView *parent)
        : q_ptr(parent)
    {
        pieSeries = new QPieSeries(q_ptr);

        chart = new QChart;
        chart->setAnimationOptions(QChart::AllAnimations);
        chart->setTitle(QObject::tr("Pie Chart"));
    }

    ChartView *q_ptr;

    QChart *chart;
    QPieSeries *pieSeries;
};

PieChart::PieChart(QWidget *parent)
    : ChartView(parent)
    , d_ptr(new PieChartPrivate(this))
{
    setupUI();
}

PieChart::~PieChart() = default;

void PieChart::setupUI()
{
    setRenderHint(QPainter::Antialiasing);
    setChart(d_ptr->chart);
    PointList pointList = generateRandomDataPoints(5, 100);
    for(int i=0; i<pointList.size(); i++){
        QPieSlice *slice = d_ptr->pieSeries->append(tr("P%1").arg(i), pointList[i].y());
        if(i == 0){
            // Show the first slice exploded with label
            slice->setLabelVisible();
            slice->setExploded();
            //slice->setExplodeDistanceFactor(0.5);
        }
    }

    d_ptr->chart->addSeries(d_ptr->pieSeries);
}
