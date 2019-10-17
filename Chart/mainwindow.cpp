#include "mainwindow.h"

#include <QtCharts>

#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFrame *frame=new QFrame(this);
    QHBoxLayout *layout=new QHBoxLayout(frame);
    layout->setContentsMargins(0, 0, 0, 0);

    QList<QPoint> point;
    point<<QPoint(0,6)<<QPoint(1,2)<<QPoint(2,10)<<QPoint(3,5)<<QPoint(4,7)
        <<QPoint(5,1)<<QPoint(6,0)<<QPoint(7,3)<<QPoint(8,1)<<QPoint(9,2)<<QPoint(10,5);

    QChart *lineChart = new QChart();
    lineChart->setTitle("折线图");
    lineChart->legend()->hide();

    QLineSeries *series = new QLineSeries(lineChart);
    for(const QPoint &data:point)
        series->append(data);
    lineChart->addSeries(series);
    lineChart->createDefaultAxes();
    lineChart->axes(Qt::Horizontal).first()->setRange(0, 10);
    lineChart->axes(Qt::Vertical).first()->setRange(0, 10);

    QValueAxis *axisY = qobject_cast<QValueAxis*>(lineChart->axes(Qt::Vertical).first());
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%.1f  ");

    QChartView *chartView = new QChartView(lineChart);
    //设置抗锯齿
    chartView->setRenderHint(QPainter::Antialiasing);

    layout->addWidget(chartView);
    setCentralWidget(frame);
    setMinimumSize(500,500);
}

MainWindow::~MainWindow()
{
}

