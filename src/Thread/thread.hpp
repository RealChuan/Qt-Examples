#ifndef THREAD_HPP
#define THREAD_HPP

#include <QRunnable>
#include <QThread>

class Task;

class Thread1 : public QThread
{
public:
    using QThread::QThread;

protected:
    void run() override;
};

class Thread2 : public QThread
{
    Q_OBJECT
public:
    using QThread::QThread;
    ~Thread2() override;

public slots:
    void onDo();

signals:
    void doo(const QString &text);

protected:
    void run() override;
};

class Thread3 : public QObject
{
    Q_OBJECT
public:
    explicit Thread3(QObject *parent = nullptr);
    ~Thread3() override;

    void dooo();

signals:
    void doo(const QString &text);

private:
    Task *m_task;
    QThread *m_thread;
};

void runThread4();

void runThread5();

auto runThread6() -> QThread *;

#endif // THREAD_HPP
