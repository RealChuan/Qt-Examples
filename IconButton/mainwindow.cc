#include "mainwindow.hpp"
#include "iconfliter.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto button = new QPushButton(tr("PushButton"), this);
    button->setCheckable(true);
    auto iconFliter = new IconFliter(button);
    iconFliter->setNormalIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    iconFliter->setHoverIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
    iconFliter->setPressedIcon(style()->standardIcon(QStyle::SP_MessageBoxCritical));

    setCentralWidget(button);
    resize(300, 185);
}

MainWindow::~MainWindow() {}
