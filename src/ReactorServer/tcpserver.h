#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class Thread;
class SubReactor;
class TcpServerPrivate;
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(int num = 10, QObject *parent = nullptr);
    ~TcpServer() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void incomingConnection(qintptr handle) override;

private:
    void createSocket(qintptr);
    auto getNextThread() -> Thread*;

    QScopedPointer<TcpServerPrivate> d;
};

#endif // TCPSERVER_H
