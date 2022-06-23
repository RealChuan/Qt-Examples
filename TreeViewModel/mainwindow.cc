#include "mainwindow.hpp"
#include "listview.hpp"
#include "normaltreeview.hpp"

#include <QtWidgets>

class MainWindow::MainWindowPrivate
{
public:
    MainWindowPrivate(QWidget *parent)
        : owner(parent)
    {
        listView = new ListView(owner);
        normalTreeView = new NormalTreeView(owner);
    }

    QWidget *owner;

    ListView *listView;
    NormalTreeView *normalTreeView;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new MainWindowPrivate(this))
{
    QString path(qApp->applicationDirPath());
    auto itemList = QDir(path).entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                                             QDir::DirsFirst);
    d_ptr->listView->setDatas(itemList);

    int count = 2;
    int index = path.lastIndexOf("/");
    while (index > 0 && count > 0) {
        path = path.left(index);
        index = path.lastIndexOf("/");
        count--;
    }
    path += "/";

    itemList = QDir(path).entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
                                        QDir::DirsFirst);
    d_ptr->normalTreeView->setDatas(itemList);

    auto tabWidget = new QTabWidget(this);
    tabWidget->addTab(d_ptr->normalTreeView, "Normal");
    tabWidget->addTab(d_ptr->listView, "ListView");

    setCentralWidget(tabWidget);

    resize(1000, 618);
}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    QMetaObject::invokeMethod(d_ptr->listView, "onResize", Qt::QueuedConnection);
    QMetaObject::invokeMethod(d_ptr->normalTreeView, "onResize", Qt::QueuedConnection);
}
