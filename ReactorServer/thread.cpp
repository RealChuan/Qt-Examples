#include "thread.h"
#include "tcpsocket.h"
#include "subreactor.h"

#include <QHostAddress>

Thread::Thread(QObject *parent)
    :QThread(parent)
{

}

Thread::~Thread()
{
    if(isRunning()){
        quit();
        wait();
    }
    qDebug() << "~Thread";
}

void Thread::run()
{
    qDebug() << "run: " << QThread::currentThreadId();
    QScopedPointer<SubReactor> subReactor(new SubReactor);
    connect(this, &Thread::newConnectHandle, subReactor.data(), &SubReactor::onNewConnect);
    connect(subReactor.data(), &SubReactor::message, this, &Thread::message);
    connect(subReactor.data(), &SubReactor::clientCount, this, &Thread::clientCount);
    connect(subReactor.data(), &SubReactor::maxCount, this, &Thread::maxCount);
    exec();
}

