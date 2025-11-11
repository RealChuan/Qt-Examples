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

    AccepterThread *accepter = nullptr;
    quint16 port = 0;
    int threadCount = 0;
    bool running = false;
};

ReactorServer::ReactorServer(quint16 port, int threadCount, QObject *parent)
    : QObject(parent)
    , d_ptr(new ReactorServerPrivate(this))
{
    d_ptr->port = port;
    d_ptr->threadCount = threadCount;

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
    if (d_ptr->running)
        return;

    d_ptr->accepter->start();
    d_ptr->running = true;
    emit message("ReactorServer started");
}

void ReactorServer::stop()
{
    if (!d_ptr->running)
        return;

    if (d_ptr->accepter && d_ptr->accepter->isRunning()) {
        d_ptr->accepter->quit();
        d_ptr->accepter->wait();
    }
    d_ptr->running = false;
    emit message("ReactorServer stopped");
}

bool ReactorServer::isRunning() const
{
    return d_ptr->running;
}
