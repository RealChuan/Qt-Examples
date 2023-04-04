#include "mainwindow.h"
#include "areachart.h"
#include "datetimechart.h"
#include "dynamicchart.h"
#include "dynamicchartx.h"
#include "linechart.h"
#include "piechart.h"
#include "stackedbarchart.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new DynamicChart(this), 0, 0);
    layout->addWidget(new DynamicChartX(this), 0, 1);
    layout->addWidget(new AreaChart(this), 0, 2);
    layout->addWidget(new DateTimeChart(this), 1, 0);
    //layout->addWidget(new LineChart(this), 1, 0);
    layout->addWidget(new StackedBarChart(this), 1, 1);
    layout->addWidget(new PieChart(this), 1, 2);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);
    setMinimumSize(960, 600);
}
