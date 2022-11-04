#include "mainwindow.hpp"
#include "passwordlineedit.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto widget = new QWidget(this);
    auto passwordLineEdit = new PasswordLineEdit(this);
    passwordLineEdit->setMinimumHeight(50);
    auto layout = new QHBoxLayout(widget);
    layout->addWidget(passwordLineEdit);
    setCentralWidget(widget);
    setMinimumSize(300, 130);
}

MainWindow::~MainWindow() {}
