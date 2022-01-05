#include "test.h"

#include <QDebug>
#include <QThread>

Test::Test(QObject *parent)
    : QObject(parent)
    , count(0)
{}

Test::~Test()
{
    qDebug() << "~Test";
}

void Test::doSomeThing(int index)
{
    const QString str = tr("I am thread %1: %2, Count: %3")
                            .arg(QString::number(index),
                                 QString::number(
                                     (reinterpret_cast<qint64>(QThread::currentThreadId()))),
                                 QString::number(count));
    emit message(str);
    count++;
}
