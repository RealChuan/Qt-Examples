#include "mainwindow.h"
#include "navigationprogressbar.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    NavigationProgressBar *nav = new NavigationProgressBar(this);
    QSlider *slider = new QSlider(this);
    slider->setRange(0, 5);
    connect(slider, &QSlider::valueChanged, nav, &NavigationProgressBar::setStep);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(nav);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(850, 150);
}

MainWindow::~MainWindow()
{
}

