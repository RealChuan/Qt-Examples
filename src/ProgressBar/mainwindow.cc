#include "mainwindow.hpp"
#include "progressbar.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QSlider *slider = new QSlider(this);
    slider->setRange(0, 100);

    ProgressBar *progressBar1 = new ProgressBar(this);
    progressBar1->setFixedHeight(15);
    progressBar1->setAutoRadius(false);
    connect(slider, &QSlider::valueChanged, progressBar1, &ProgressBar::valueChanged);

    ProgressBar *progressBar2 = new ProgressBar(this);
    progressBar2->setFixedHeight(15);
    progressBar2->setAutoRadius(true);
    progressBar2->setChunkColor(QColor(250, 118, 113));
    connect(slider, &QSlider::valueChanged, progressBar2, &ProgressBar::valueChanged);

    ProgressBar *progressBar3 = new ProgressBar(this);
    progressBar3->setFixedHeight(15);
    progressBar3->setRadius(5);
    progressBar3->setChunkColor(QColor(41, 197, 90));
    connect(slider, &QSlider::valueChanged, progressBar3, &ProgressBar::valueChanged);

    QVBoxLayout *progressBarLayout = new QVBoxLayout;
    progressBarLayout->addWidget(progressBar1);
    progressBarLayout->addWidget(progressBar2);
    progressBarLayout->addWidget(progressBar3);

    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addLayout(progressBarLayout);
    layout->addWidget(slider);
    setCentralWidget(widget);
    resize(670, 260);
}

MainWindow::~MainWindow()
{
}

