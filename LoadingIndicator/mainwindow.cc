#include "mainwindow.hpp"
#include "loadingindicator.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *button = new QPushButton(tr("Loading Indicator Button: Show 10 seconds"), this);
    connect(button, &QPushButton::clicked, this, &MainWindow::onShowLoadingIndicator);
    setCentralWidget(button);

    resize(300, 185);
}

MainWindow::~MainWindow() {}

void MainWindow::onShowLoadingIndicator()
{
    auto *loadingIndicator = new LoadingIndicator(this);
    QTimer::singleShot(10000, loadingIndicator, &LoadingIndicator::deleteLater);
}
