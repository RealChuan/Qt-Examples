#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

class WorkerThread;
class ThreadPool : public QObject
{
    Q_OBJECT
public:
    explicit ThreadPool(int count,
                        const ConnectionCallbacks &callbacks = {},
                        QObject *parent = nullptr);
    ~ThreadPool();

    void dispatchConnection(qintptr socketDescriptor);
    int activeThreadCount() const;

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

private:
    class ThreadPoolPrivate;
    QScopedPointer<ThreadPoolPrivate> d_ptr;
};
