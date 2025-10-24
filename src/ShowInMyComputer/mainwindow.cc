#include "mainwindow.hpp"
#include "showinmycomputer.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto checkBox = new QCheckBox(tr("ShowInMyComputerme"), this);
    checkBox->setChecked(ShowInMyComputerme::isShowInMyComputer());
    connect(checkBox, &QCheckBox::stateChanged, checkBox, [checkBox] {
        ShowInMyComputerme::setShowInMyComputer(checkBox->isChecked());
    });

    setCentralWidget(checkBox);
}

MainWindow::~MainWindow() {}
