#include "MyThread_object.h"
#include "MyThread.h"

#include <QDebug>

#pragma execution_character_set("utf-8")

MyObjectThread::MyObjectThread()
    :started(true)
{
    connect(this,&MyObjectThread::selfSignal,this,&MyObjectThread::dosomething);
}

void MyObjectThread::test()
{
    qDebug()<<"test: "<<QThread::currentThreadId();
}

void MyObjectThread::setRun(bool f)
{
    started=f;
}

void MyObjectThread::dosomething()
{
    for(int i=0;i<1000;i++)
    {
        if(!started)
            return;
        qDebug()<<"dosomething："<<QThread::currentThreadId();
        emit message(i);
        QThread::msleep(1000);
    }
}

void MyObjectThread::closeThread()
{
    started=false;
}
