#include "reactorserver.hpp"
#include "accepterthread.hpp"

#include <QDebug>

class ReactorServer::ReactorServerPrivate
{
public:
    explicit ReactorServerPrivate(ReactorServer *q)
        : q_ptr(q)
    {}

    ReactorServer *q_ptr;

    AccepterThread *accepter;
};

ReactorServer::ReactorServer(quint16 port, int threadCount, QObject *parent)
    : QObject(parent)
    , d_ptr(new ReactorServerPrivate(this))
{
    d_ptr->accepter = new AccepterThread(port, threadCount, this);
    connect(d_ptr->accepter, &AccepterThread::message, this, &ReactorServer::message);
    connect(d_ptr->accepter,
            &AccepterThread::clientCountChanged,
            this,
            &ReactorServer::clientCountChanged);
    connect(d_ptr->accepter, &AccepterThread::maxClientCount, this, &ReactorServer::maxClientCount);
}

ReactorServer::~ReactorServer()
{
    stop();
}

void ReactorServer::setCallbacks(const ConnectionCallbacks &callbacks)
{
    d_ptr->accepter->setCallbacks(callbacks);
}

void ReactorServer::start()
{
    if (d_ptr->accepter->isRunning()) {
        return;
    }

    d_ptr->accepter->start();
    emit message(tr("ReactorServer started"));
}

void ReactorServer::stop()
{
    if (!d_ptr->accepter->isRunning()) {
        return;
    }

    d_ptr->accepter->quit();
    d_ptr->accepter->wait();
    emit message(tr("ReactorServer stopped"));
}

bool ReactorServer::isRunning() const
{
    return d_ptr->accepter->isRunning();
}
