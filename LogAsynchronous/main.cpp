#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>

#include "logasync.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    LogAsync log;
    log.setLogLevel(QtDebugMsg);
    log.start();

    QElapsedTimer timer;
    timer.start();

    for(int i=0; i< 1000 * 1000; i++){
        qDebug() << "123456789qwertyuioplkjhgfdsa" << i;
    }

    qDebug() << timer.elapsed();

    return 0;
}
