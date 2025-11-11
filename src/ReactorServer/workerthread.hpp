#pragma once

#include "connectioncallbacks.hpp"

#include <QThread>

#include <memory>
#include <vector>

class SubReactor;
class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(const ConnectionCallbacks &callbacks = {}, QObject *parent = nullptr);
    ~WorkerThread();

    void handleConnection(qintptr socketDescriptor);
    int clientCount() const;

signals:
    void message(const QString &msg);
    void clientConnected();
    void clientDisconnected();

protected:
    void run() override;

private:
    class WorkerThreadPrivate;
    QScopedPointer<WorkerThreadPrivate> d_ptr;
};

using WorkerThreadPtr = std::unique_ptr<WorkerThread>;
using WorkerThreadList = std::vector<WorkerThreadPtr>;
