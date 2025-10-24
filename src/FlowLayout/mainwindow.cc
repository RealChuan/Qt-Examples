#include "mainwindow.hpp"
#include "flowwidget.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QStringList list;
    for (int i = 0; i < 100; i++) {
        if (i % 5 == 1) {
            list.append(QString::number(QRandomGenerator::global()->generate64()));
        } else {
            list.append(QString::number(QRandomGenerator::global()->generate()));
        }
    }

    FlowWidget *flowWidget = new FlowWidget(this);
    flowWidget->setItems(list);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(flowWidget);
    setCentralWidget(scrollArea);
    resize(600, 320);
}

MainWindow::~MainWindow()
{
}

