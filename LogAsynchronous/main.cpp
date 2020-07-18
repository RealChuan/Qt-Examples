#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>

#include "logasync.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    LogAsync *log = LogAsync::instance();
    log->setLogLevel(QtDebugMsg); // 实际环境中可通过读取配置设置日志级别
    log->startWork();

    QElapsedTimer timer;
    timer.start();

    for(int i=0; i< 1000 * 1000; i++){
        qInfo() << "1234567890qwertyuiopasdfghjklzxcvbnm" << i;
    }

    qInfo() << timer.elapsed();
    log->finish();
    return 0;
}
