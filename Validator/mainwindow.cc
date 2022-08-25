#include "mainwindow.hpp"
#include "validator.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto lineEdit1 = new QLineEdit(this);
    lineEdit1->setValidator(new QIntValidator(1, 199, lineEdit1));
    auto lineEdit2 = new QLineEdit(this);
    lineEdit2->setValidator(new QDoubleValidator(1, 199, 2, lineEdit2));
    auto box1 = new QGroupBox("QValidator", this);
    auto layout1 = new QHBoxLayout(box1);
    layout1->addWidget(lineEdit1);
    layout1->addWidget(lineEdit2);

    auto lineEdit3 = new QLineEdit(this);
    lineEdit3->setValidator(new IntValidator(1, 199, lineEdit3));
    auto lineEdit4 = new QLineEdit(this);
    lineEdit4->setValidator(new DoubleValidator(1, 199, 2, lineEdit4));
    auto box2 = new QGroupBox("Validator", this);
    auto layout2 = new QHBoxLayout(box2);
    layout2->addWidget(lineEdit3);
    layout2->addWidget(lineEdit4);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->addWidget(box1);
    layout->addWidget(box2);
    setCentralWidget(widget);
    resize(300, 300 * 0.618);
}

MainWindow::~MainWindow() {}
