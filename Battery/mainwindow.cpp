#include "mainwindow.h"
#include "batterywidget.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *battery = new BatteryWidget(this);
    auto *slider = new QSlider(this);
    slider->setRange(0, 100);
    connect(slider, &QSlider::valueChanged, battery, &BatteryWidget::valueChanged);

    auto *widget = new QWidget(this);
    auto *layout = new QHBoxLayout(widget);
    layout->addWidget(battery);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(300, 160);
}

MainWindow::~MainWindow() {}
