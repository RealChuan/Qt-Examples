#include "mainwindow.h"

#include "bubblewidget.h"

#include <QtWidgets>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    resize(370, 600);
}

void MainWindow::onShowBubble()
{
    QPoint point = mapToGlobal(button->pos());

    int x = point.x() + button->width() / 2;
    int y = point.y() + button->height() /2;

    BubbleWidget w(this);
    w.setTriangleInfo(20, 10);
    w.setDerection(BubbleWidget::Top);
    w.setCentralWidget(new QLabel("display", this));
    w.move(x - w.width() / 2, y - SHADOW_WIDTH);
    w.exec();
}

void MainWindow::setupUI()
{
    button = new QPushButton("show", this);
    connect(button, &QPushButton::clicked, this, &MainWindow::onShowBubble);

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
    layout->addWidget(button);
    layout->addWidget(top);
    layout->addWidget(left);
    layout->addWidget(right);
    layout->addWidget(bottom);

    setCentralWidget(widget);
}

