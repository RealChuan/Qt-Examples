#include "tcpserver.h"
#include "subreactor.h"
#include "thread.h"

class TcpServerPrivate
{
public:
    explicit TcpServerPrivate(QObject *parent)
        : owner(parent)
    {}
    QObject *owner;
    QVector<Thread *> threads;
    SubReactor *subReactor = nullptr;
    int index = 0;
};

TcpServer::TcpServer(int num, QObject *parent)
    : QTcpServer(parent)
    , d(new TcpServerPrivate(this))
{
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<QAtomicInt>("QAtomicInt");

    for (int i = 0; i < num; i++) { // one Loop Thread
        Thread *thread = new Thread(this);
        connect(thread, &Thread::message, this, &TcpServer::message);
        connect(thread, &Thread::maxCount, this, &TcpServer::maxCount);
        connect(thread, &Thread::clientCount, this, &TcpServer::clientCount);
        d->threads.append(thread);
        thread->start();
    }

    if (num <= 0) { // one Thread in Accepter
        d->subReactor = new SubReactor(this);
        connect(d->subReactor, &SubReactor::message, this, &TcpServer::message);
        connect(d->subReactor, &SubReactor::maxCount, this, &TcpServer::maxCount);
        connect(d->subReactor, &SubReactor::clientCount, this, &TcpServer::clientCount);
    }
}

TcpServer::~TcpServer()
{
    if (!d->threads.isEmpty()) {
        qDeleteAll(d->threads);
        d->threads.clear();
    }
    close();
    qDebug() << "~TcpServer";
}

//-------------------------------------------------
//                            Accepter
//-------------------------------------------------
void TcpServer::incomingConnection(qintptr handle)
{
    auto thread = getNextThread();
    if (thread != nullptr) {
        emit thread->newConnectHandle(handle);
    } else if (d->subReactor != nullptr) {
        d->subReactor->onNewConnect(handle);
    }
}

auto TcpServer::getNextThread() -> Thread *
{
    if (d->threads.isEmpty()) {
        return nullptr;
    }
    if (d->index < 0 || d->index >= d->threads.size())
        d->index = 0;
    Thread *thread = d->threads.at(d->index);
    d->index++;
    return thread;
}
