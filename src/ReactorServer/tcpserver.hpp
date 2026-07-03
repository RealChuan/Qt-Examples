#pragma once

#include "connectioncallbacks.hpp"

#include <QTcpServer>

#include <memory>

class ThreadPool;

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(int threadCount,
                       const ConnectionCallbacks &callbacks = {},
                       QObject *parent = nullptr);
    ~TcpServer();

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

protected:
    void incomingConnection(qintptr handle) override;

private:
    class TcpServerPrivate;
    std::unique_ptr<TcpServerPrivate> d_ptr;
};
