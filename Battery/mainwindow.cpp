#include "mainwindow.h"
#include "batterywidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    BatteryWidget *battery = new BatteryWidget(this);
    QSlider *slider = new QSlider(this);
    slider->setRange(0, 100);
    connect(slider, &QSlider::valueChanged, battery, &BatteryWidget::valueChanged);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(battery);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(300, 160);
}

MainWindow::~MainWindow() {}
