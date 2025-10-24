#include "mainwindow.hpp"

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
