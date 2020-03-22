#include "thread2.h"
#include "test.h"

Thread2::Thread2(QObject *parent)
    :QThread(parent)
{
}

Thread2::~Thread2()
{
    if(isRunning()){
        quit();
        wait();
    }
}

void Thread2::run()
{
    QScopedPointer<Test> test(new Test);
    connect(this, &Thread2::doSomeThing, test.data(), &Test::doSomeThing);
    connect(test.data(), &Test::message, this, &Thread2::message, Qt::DirectConnection);
    exec();
}
