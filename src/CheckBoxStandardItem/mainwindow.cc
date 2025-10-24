#include "mainwindow.hpp"
#include "checkboxstandarditem.hpp"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto treeWidget = new QTreeView(this);
    treeWidget->header()->hide();
    auto model = new QStandardItemModel(this);
    treeWidget->setModel(model);

    for (int i = 0; i < 5; i++) {
        auto parent = new CheckBoxStandardItem(QString::number(i));
        for (int j = 0; j < 5; j++) {
            auto child = new CheckBoxStandardItem(QString::number(j));
            parent->appendRow(child);
        }
        model->appendRow(parent);
    }

    setCentralWidget(treeWidget);

    treeWidget->expandAll();
    resize(500, 500 * 0.618);
}

MainWindow::~MainWindow() {}
