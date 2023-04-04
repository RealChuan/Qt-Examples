#include "mainwindow.h"
#include "clockwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ClockWidget *clockWidget = new ClockWidget(this);
    setCentralWidget(clockWidget);
    resize(600, 480);
}

MainWindow::~MainWindow() = default;
