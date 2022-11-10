#include "thread.hpp"
#include "functions.hpp"
#include "task.hpp"

#include <QDebug>
#include <QThreadPool>
#include <QtConcurrent>

void Thread1::run()
{
    qDebug() << "Thread1----" << Functions::getCurrentThreadIDStr();
}

Thread2::~Thread2()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void Thread2::onDo()
{
    emit doo("Thread2----");
}

void Thread2::run() // 类似 moveToThread
{
    QScopedPointer<Task> taskPtr(new Task);
    connect(this, &Thread2::doo, taskPtr.data(), &Task::onDo);
    exec();
}

Thread3::Thread3(QObject *parent)
    : QObject(parent)
    , m_task(new Task)
    , m_thread(new QThread)
{
    m_task->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_task, &Task::deleteLater);
    connect(this, &Thread3::doo, m_task, &Task::onDo);
    m_thread->start();
}

Thread3::~Thread3()
{
    if (m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait();
    }
}

void Thread3::dooo()
{
    emit doo("Thread3----");
}

class Runnable : public QRunnable
{
public:
    using QRunnable::QRunnable;

protected:
    void run() override { qDebug() << "Thread4----" << Functions::getCurrentThreadIDStr(); }
};

void runThread4()
{
    QThreadPool::globalInstance()->start(new Runnable);
}

void runThread5()
{
    auto ret = QtConcurrent::run(
        [] { qDebug() << "Thread5----" << Functions::getCurrentThreadIDStr(); });
}

QThread *runThread6()
{
    return QThread::create([] { qDebug() << "Thread6----" << Functions::getCurrentThreadIDStr(); });
}
