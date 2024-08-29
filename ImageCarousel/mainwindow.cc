#include "mainwindow.hpp"
#include "imagecarousel.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *imageViewer = new ImageCarousel(this);

    auto *previousButton = new QPushButton("&Previous", this);
    auto *nextButton = new QPushButton("&Next", this);
    connect(previousButton, &QPushButton::clicked, imageViewer, &ImageCarousel::onPrevious);
    connect(nextButton, &QPushButton::clicked, imageViewer, &ImageCarousel::onNext);

    auto *widget = new QWidget(this);
    auto *layout = new QHBoxLayout(widget);
    layout->addWidget(previousButton);
    layout->addWidget(imageViewer);
    layout->addWidget(nextButton);

    setCentralWidget(widget);

    resize(1000, 618);
}

MainWindow::~MainWindow() {}
