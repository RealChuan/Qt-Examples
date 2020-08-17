#include "mainwindow.h"
#include "dashboardwidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QSlider *slider = new QSlider(this);
    slider->setRange(0, 100);
    DashBoardWidget *dashBoardWidget = new DashBoardWidget(this);
    connect(slider, &QSlider::valueChanged, dashBoardWidget, &DashBoardWidget::valueChanged);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(dashBoardWidget);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(500, 400);
}

MainWindow::~MainWindow()
{
}

