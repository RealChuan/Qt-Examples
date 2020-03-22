#include "thread1.h"

Thread1::Thread1(QObject *parent)
    :QThread(parent)
    ,started(true)
    ,count(0)
{
}

Thread1::~Thread1()
{
    started = false;
    blockSignals(true);
    condition.wakeOne();
    if(isRunning()){
        quit();
        wait();
    }
}

void Thread1::wakeUpThread()
{
    condition.wakeOne();
}

void Thread1::run()
{
    while(started){
        QMutexLocker lock(&mutex);
        condition.wait(&mutex);
        QString str = tr("I am thread 1: %1, Count: %2").
                arg(reinterpret_cast<int>(QThread::currentThreadId())).
                arg(count);
        emit message(str);
        count++;
    }
}
