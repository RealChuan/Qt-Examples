#include "mainwindow.hpp"
#include "areachart.hpp"
#include "datetimechart.hpp"
#include "dynamicchart.hpp"
#include "dynamicchartx.hpp"
#include "linechart.hpp"
#include "piechart.hpp"
#include "stackedbarchart.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{ setupUI(); }

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
