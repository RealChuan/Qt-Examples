#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "logasync.h"
#include "mainwindow.h"

auto main(int argc, char *argv[]) -> int
{
    QApplication a(argc, argv);

    LogAsync *log = LogAsync::instance();
    log->setOrientation(LogAsync::Orientation::StdAndFile);
    log->setLogLevel(QtDebugMsg);
    log->startWork();

    //    QElapsedTimer timer;
    //    timer.start();

    //    for(int i=0; i< 1000 * 1000; i++){
    //        qInfo() << "1234567890qwertyuiopasdfghjklzxcvbnm" << i;
    //    }

    //    qInfo() << timer.elapsed();

    MainWindow w;
    w.show();

    int result = a.exec();
    log->stop();
    return result;
}
