#include <QApplication>

#include "mainwindow.h"

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    int result = a.exec();
    return result;
}
