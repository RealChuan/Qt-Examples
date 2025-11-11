#include "tcpserver.hpp"
#include "threadpool.hpp"

#include <QDebug>

class TcpServer::TcpServerPrivate
{
public:
    explicit TcpServerPrivate(TcpServer *q)
        : q_ptr(q)
    {}

    TcpServer *q_ptr;

    ThreadPool *threadPool = nullptr;
    QAtomicInteger<qint32> totalClientCount{0};
    QAtomicInteger<qint32> maxClientCount{0};
};

TcpServer::TcpServer(int threadCount, const ConnectionCallbacks &callbacks, QObject *parent)
    : QTcpServer(parent)
    , d_ptr(new TcpServerPrivate(this))
{
    qRegisterMetaType<qintptr>("qintptr");

    d_ptr->threadPool = new ThreadPool(threadCount, callbacks, this);
    connect(d_ptr->threadPool, &ThreadPool::message, this, &TcpServer::message);
    connect(d_ptr->threadPool,
            &ThreadPool::clientCountChanged,
            this,
            &TcpServer::clientCountChanged);
    connect(d_ptr->threadPool, &ThreadPool::maxClientCount, this, &TcpServer::maxClientCount);
}

TcpServer::~TcpServer()
{
    close();
}

void TcpServer::incomingConnection(qintptr handle)
{
    d_ptr->threadPool->dispatchConnection(handle);
}
