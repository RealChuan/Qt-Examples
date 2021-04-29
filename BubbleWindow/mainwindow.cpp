#include "mainwindow.h"

#include "bubblewidget.h"

#include <QtWidgets>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    resize(370, 400);
}

void MainWindow::setupUI()
{
    BubbleWidget *top = new BubbleWidget(this);
    top->setTriangleInfo(20, 20);
    top->setDerection(BubbleWidget::Top);

    BubbleWidget *left = new BubbleWidget(this);
    left->setTriangleInfo(20, 20);
    left->setDerection(BubbleWidget::Left);

    BubbleWidget *right = new BubbleWidget(this);
    right->setTriangleInfo(20, 20);
    right->setDerection(BubbleWidget::Right);

    BubbleWidget *bottom = new BubbleWidget(this);
    bottom->setTriangleInfo(20, 20);
    bottom->setDerection(BubbleWidget::Bottom);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(top);
    layout->addWidget(left);
    layout->addWidget(right);
    layout->addWidget(bottom);

    setCentralWidget(widget);
}

