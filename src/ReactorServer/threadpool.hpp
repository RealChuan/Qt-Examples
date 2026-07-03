#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

#include <memory>

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
    [[nodiscard]] int activeThreadCount() const;

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

private:
    class ThreadPoolPrivate;
    std::unique_ptr<ThreadPoolPrivate> d_ptr;
};
