#include "accepterthread.hpp"
#include "tcpserver.hpp"

#include <QDebug>

#include <memory>

using namespace Qt::StringLiterals;

class AccepterThread::AccepterThreadPrivate
{
public:
    explicit AccepterThreadPrivate(AccepterThread *q) : q_ptr(q) {}

    AccepterThread *q_ptr;

    quint16 port = 0;
    int threadCount = 0;
    ConnectionCallbacks callbacks;
};

AccepterThread::AccepterThread(quint16 port, int threadCount, QObject *parent)
    : QThread(parent), d_ptr(new AccepterThreadPrivate(this))
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
{ d_ptr->callbacks = callbacks; }

void AccepterThread::run()
{
    const auto tcpServer = std::make_unique<TcpServer>(d_ptr->threadCount, d_ptr->callbacks);

    connect(tcpServer.get(), &TcpServer::message, this, &AccepterThread::message);
    connect(
        tcpServer.get(), &TcpServer::clientCountChanged, this, &AccepterThread::clientCountChanged);
    connect(tcpServer.get(), &TcpServer::maxClientCount, this, &AccepterThread::maxClientCount);

    if (!tcpServer->listen(QHostAddress::Any, d_ptr->port)) {
        qCritical() << u"Failed to start server:"_s << tcpServer->errorString();
        return;
    }

    emit message(u"Server started on port %1"_s.arg(d_ptr->port));
    exec();
}
