#ifndef MYOBJECTTHREAD_H
#define MYOBJECTTHREAD_H

#include <QObject>

class MyObjectThread: public QObject
{
    Q_OBJECT

public:
    MyObjectThread();

    void test();
    void setRun(bool f);
    void dosomething();
    void closeThread();

signals:
    void selfSignal();
    void message(int);

private:
    volatile bool started;       //isStop是易失性变量，需要用volatile进行申明
};

#endif // MYOBJECTTHREAD_H
