#include "workerthread.hpp"
#include "subreactor.hpp"

#include <QDebug>

class WorkerThread::WorkerThreadPrivate
{
public:
    explicit WorkerThreadPrivate(WorkerThread *q) : q_ptr(q) {}

    WorkerThread *q_ptr;

    QPointer<SubReactor> subReactorPtr;
    ConnectionCallbacks callbacks;
};

WorkerThread::WorkerThread(const ConnectionCallbacks &callbacks, QObject *parent)
    : QThread(parent), d_ptr(std::make_unique<WorkerThreadPrivate>(this))
{ d_ptr->callbacks = callbacks; }

WorkerThread::~WorkerThread()
{
    if (isRunning()) {
        quit();
        wait();
    }
}

void WorkerThread::run()
{
    const auto subReactor = std::make_unique<SubReactor>(d_ptr->callbacks);

    d_ptr->subReactorPtr = subReactor.get();

    connect(subReactor.get(), &SubReactor::message, this, &WorkerThread::message);
    connect(subReactor.get(), &SubReactor::clientConnected, this, &WorkerThread::clientConnected);
    connect(
        subReactor.get(), &SubReactor::clientDisconnected, this, &WorkerThread::clientDisconnected);

    exec();
}

void WorkerThread::handleConnection(qintptr socketDescriptor)
{
    if (d_ptr->subReactorPtr) {
        QMetaObject::invokeMethod(
            d_ptr->subReactorPtr.data(),
            [this, socketDescriptor]() { d_ptr->subReactorPtr->addConnection(socketDescriptor); },
            Qt::QueuedConnection);
    }
}

int WorkerThread::clientCount() const
{ return d_ptr->subReactorPtr ? d_ptr->subReactorPtr->clientCount() : 0; }
