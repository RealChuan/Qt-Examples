#include "threadpool.hpp"
#include "workerthread.hpp"

#include <QDebug>

class ThreadPool::ThreadPoolPrivate
{
public:
    explicit ThreadPoolPrivate(ThreadPool *q)
        : q_ptr(q)
    {}

    ~ThreadPoolPrivate()
    {
        for (auto &thread : threads) {
            if (thread->isRunning()) {
                thread->quit();
                thread->wait();
            }
        }
        threads.clear();

        qInfo() << "Total connected clients: " << totalConnectedClients.load();
    }

    void updateClientCount(int change)
    {
        if (change > 0) {
            totalConnectedClients.fetch_add(change);
        }

        qint32 oldCount = totalClientCount.fetch_add(change);
        qint32 newCount = oldCount + change;

        // 使用CAS循环来确保正确更新最大值
        qint32 currentMax = maxClientCount.load();
        while (newCount > currentMax) {
            if (maxClientCount.compare_exchange_weak(currentMax, newCount)) {
                emit q_ptr->maxClientCount(newCount);
                break;
            }
        }
        emit q_ptr->clientCountChanged(newCount);
    }

    ThreadPool *q_ptr;

    WorkerThreadList threads;
    int currentIndex = 0;
    std::atomic<qint32> totalClientCount{0};
    std::atomic<qint32> maxClientCount{0};
    std::atomic<qint32> totalConnectedClients{0};
};

ThreadPool::ThreadPool(int count, const ConnectionCallbacks &callbacks, QObject *parent)
    : QObject(parent)
    , d_ptr(new ThreadPoolPrivate(this))
{
    for (int i = 0; i < count; ++i) {
        auto *thread = new WorkerThread(callbacks);

        connect(thread, &WorkerThread::message, this, &ThreadPool::message);
        connect(thread, &WorkerThread::clientConnected, this, [this]() {
            d_ptr->updateClientCount(1);
        });
        connect(thread, &WorkerThread::clientDisconnected, this, [this]() {
            d_ptr->updateClientCount(-1);
        });

        thread->start();
        d_ptr->threads.emplace_back(thread);
    }
}

ThreadPool::~ThreadPool() {}

void ThreadPool::dispatchConnection(qintptr socketDescriptor)
{
    if (d_ptr->threads.empty())
        return;

    // Simple round-robin load balancing
    auto &thread = d_ptr->threads[d_ptr->currentIndex];
    d_ptr->currentIndex = (d_ptr->currentIndex + 1) % d_ptr->threads.size();

    thread->handleConnection(socketDescriptor);
}

int ThreadPool::activeThreadCount() const
{
    return static_cast<int>(d_ptr->threads.size());
}
