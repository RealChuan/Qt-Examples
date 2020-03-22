#include "thread3.h"
#include "test.h"

#include <QThread>

Thread3::Thread3(QObject *parent)
    : QObject(parent)
    , test(new Test)
{
    thread = new QThread(this);
    test->moveToThread(thread);
    //connect(thread, &QThread::destroyed, test.data(), &Test::deleteLater);
    connect(this, &Thread3::doSomething, test.data(), &Test::doSomeThing);
    connect(test.data(), &Test::message, this, &Thread3::message, Qt::DirectConnection);
    thread->start();
}

Thread3::~Thread3()
{
    if(thread->isRunning()){
        thread->quit();
        thread->wait();
    }
}

//void Thread3::doSomeThing(int index)
//{
//    QMetaObject::invokeMethod(test, "doSomeThing",
//                              Qt::QueuedConnection,
//                              Q_ARG(int, index));
//}
