#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QCoreApplication>
#include <QThread>
#include <QTime>

class MyThread: public QThread
{
    Q_OBJECT

public:
    MyThread();
    void closeThread();
    void setRun(bool f);

protected:
    void run();     //QThread只有run函数是在新线程里的

signals:
    void message(int);

private:
    volatile bool started;       //isStop是易失性变量，需要用volatile进行申明
};

#endif // MYTHREAD_H
