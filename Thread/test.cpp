#include "test.h"

#include <QThread>
#include <QDebug>

Test::Test(QObject *parent)
    : QObject(parent)
    , count(0)
{

}

Test::~Test()
{
    qDebug() << "~Test";
}

void Test::doSomeThing(int index)
{
    QString str = tr("I am thread %1: %2, Count: %3").arg(index).
            arg(reinterpret_cast<int>(QThread::currentThreadId())).
            arg(count);
    emit message(str);
    count++;
}
