#include "mainwindow.h"
#include "dynamicchart.h"
#include "dynamicchartx.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new DynamicChart(this), 0, 0);
    layout->addWidget(new DynamicChartX(this), 0, 1);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);
    setCentralWidget(frame);
    setMinimumSize(1280, 720);
}

