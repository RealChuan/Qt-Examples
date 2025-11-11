#include "accepterthread.hpp"
#include "tcpserver.hpp"

#include <QDebug>

class AccepterThread::AccepterThreadPrivate
{
public:
    explicit AccepterThreadPrivate(AccepterThread *q)
        : q_ptr(q)
    {}

    AccepterThread *q_ptr;

    quint16 port = 0;
    int threadCount = 0;
    ConnectionCallbacks callbacks;
};

AccepterThread::AccepterThread(quint16 port, int threadCount, QObject *parent)
    : QThread(parent)
    , d_ptr(new AccepterThreadPrivate(this))
{
    d_ptr->port = port;
    d_ptr->threadCount = threadCount;
}

AccepterThread::~AccepterThread()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void AccepterThread::setCallbacks(const ConnectionCallbacks &callbacks)
{
    d_ptr->callbacks = callbacks;
}

void AccepterThread::run()
{
    QScopedPointer<TcpServer> tcpServerPtr(new TcpServer(d_ptr->threadCount, d_ptr->callbacks));

    connect(tcpServerPtr.data(), &TcpServer::message, this, &AccepterThread::message);
    connect(tcpServerPtr.data(),
            &TcpServer::clientCountChanged,
            this,
            &AccepterThread::clientCountChanged);
    connect(tcpServerPtr.data(), &TcpServer::maxClientCount, this, &AccepterThread::maxClientCount);

    if (!tcpServerPtr->listen(QHostAddress::Any, d_ptr->port)) {
        emit message("Failed to start server: " + tcpServerPtr->errorString());
        return;
    }

    emit message(QString("Server started on port %1").arg(d_ptr->port));
    exec();
}
