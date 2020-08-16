#include "mainwindow.h"
#include "slipbutton.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    SlipButton *slipButton = new SlipButton(this);
    //slipButton->setButtonHeight(50);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(slipButton);
    setCentralWidget(widget);
    resize(240, 110);
}

MainWindow::~MainWindow()
{
}

