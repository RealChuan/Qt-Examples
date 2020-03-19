#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class Thread;
class SubReactor;
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    TcpServer(int num = 10, QObject *parent = nullptr);
    ~TcpServer() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void incomingConnection(qintptr handle) override;

private:
    void createSocket(qintptr);
    Thread* getNextThread();

    QList<Thread*> threads;
    SubReactor* subReactor = nullptr;
    int index;
};

#endif // TCPSERVER_H
