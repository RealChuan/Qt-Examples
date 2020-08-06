#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>

#include "logasync.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LogAsync *log = LogAsync::instance();
    log->setLogLevel(QtDebugMsg); // 实际环境中可通过读取配置设置日志级别
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
