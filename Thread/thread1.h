#ifndef THREAD1_H
#define THREAD1_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class Thread1 : public QThread
{
    Q_OBJECT
public:
    Thread1(QObject *parent = nullptr);
    ~Thread1() override;

    void wakeUpThread();

signals:
    void message(const QString&);

protected:
    void run() override;

private:
    volatile bool started;
    QMutex mutex;
    QWaitCondition condition;
    int count;
};

#endif // THREAD1_H
