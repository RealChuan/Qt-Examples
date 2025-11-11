#pragma once

#include "connectioncallbacks.hpp"

#include <QThread>

class TcpServer;
class AccepterThread : public QThread
{
    Q_OBJECT
public:
    explicit AccepterThread(quint16 port, int threadCount, QObject *parent = nullptr);
    ~AccepterThread();

    void setCallbacks(const ConnectionCallbacks &callbacks);

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

protected:
    void run() override;

private:
    class AccepterThreadPrivate;
    QScopedPointer<AccepterThreadPrivate> d_ptr;
};
