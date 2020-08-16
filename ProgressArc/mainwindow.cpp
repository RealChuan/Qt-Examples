#include "mainwindow.h"
#include "progressarc.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QSlider *slider = new QSlider(this);
    slider->setRange(0, 100);

    ProgressArc *progressArc1 = new ProgressArc(tr("Succeed1"), this);
    connect(slider, &QSlider::valueChanged, progressArc1, &ProgressArc::valueChanged);

    ProgressArc *progressArc2 = new ProgressArc(tr("Succeed2"), this);
    progressArc2->setArcColor(QColor(250, 118, 113));
    progressArc2->setStartAngle(-50);
    progressArc2->setEndAngle(230);
    connect(slider, &QSlider::valueChanged, progressArc2, &ProgressArc::valueChanged);

    ProgressArc *progressArc3 = new ProgressArc(tr("Succeed3"), this);
    progressArc3->setArcColor(QColor(41, 197, 90));
    progressArc3->setStartAngle(-140);
    progressArc3->setEndAngle(140);
    connect(slider, &QSlider::valueChanged, progressArc3, &ProgressArc::valueChanged);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(progressArc1);
    layout->addWidget(progressArc2);
    layout->addWidget(progressArc3);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(670, 260);
}

MainWindow::~MainWindow()
{
}

