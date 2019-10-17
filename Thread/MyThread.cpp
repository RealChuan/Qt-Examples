#include "MyThread.h"

#include <QDebug>

#pragma execution_character_set("utf-8")

MyThread::MyThread()
    :started(false)
{
}

void MyThread::closeThread()
{
    started=false;
}

void MyThread::setRun(bool f)
{
    started=f;
}

void MyThread::run()
{
    for(int i=0;i<1000;i++)
    {
        if(!started)
            return;
        qDebug()<<"子线程："<<QThread::currentThreadId();
        emit message(i);
        msleep(1000);
    }
}
