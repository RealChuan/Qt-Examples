#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void incomingConnection(qintptr handle) override;
};

#endif // TCPSERVER_H
