#include "mainwindow.h"
#include "mainwindow_object.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    MainWindow_object w;
    w.show();
    return a.exec();
}
