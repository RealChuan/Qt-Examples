#include "mainwindow.h"

#include "bubblewidget.h"

#include <QDebug>
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    resize(370, 400);
}

void MainWindow::setupUI()
{
    auto *top = new BubbleWidget(this);
    top->setTriangleSize({20, 20});
    top->setDerection(BubbleWidget::Top);

    auto *left = new BubbleWidget(this);
    left->setTriangleSize({20, 20});
    left->setDerection(BubbleWidget::Left);

    auto *right = new BubbleWidget(this);
    right->setTriangleSize({20, 20});
    right->setDerection(BubbleWidget::Right);

    auto *bottom = new BubbleWidget(this);
    bottom->setTriangleSize({20, 20});
    bottom->setDerection(BubbleWidget::Bottom);

    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->addWidget(top);
    layout->addWidget(left);
    layout->addWidget(right);
    layout->addWidget(bottom);

    setCentralWidget(widget);
}
