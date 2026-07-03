#include "threadpool.hpp"
#include "workerthread.hpp"

#include <QDebug>

class ThreadPool::ThreadPoolPrivate
{
public:
    explicit ThreadPoolPrivate(ThreadPool *q) : q_ptr(q) {}

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
    std::atomic<int> currentIndex{0};
    std::atomic<qint32> totalClientCount{0};
    std::atomic<qint32> maxClientCount{0};
    std::atomic<qint32> totalConnectedClients{0};
};

ThreadPool::ThreadPool(int count, const ConnectionCallbacks &callbacks, QObject *parent)
    : QObject(parent), d_ptr(std::make_unique<ThreadPoolPrivate>(this))
{
    for (int i = 0; i < count; ++i) {
        auto thread = std::make_unique<WorkerThread>(callbacks);

        connect(thread.get(), &WorkerThread::message, this, &ThreadPool::message);
        connect(thread.get(), &WorkerThread::clientConnected, this, [this]() {
            d_ptr->updateClientCount(1);
        });
        connect(thread.get(), &WorkerThread::clientDisconnected, this, [this]() {
            d_ptr->updateClientCount(-1);
        });

        thread->start();
        d_ptr->threads.push_back(std::move(thread));
    }
}

ThreadPool::~ThreadPool() {}

void ThreadPool::dispatchConnection(qintptr socketDescriptor)
{
    if (d_ptr->threads.empty())
        return;

    // Atomic round-robin: fetch_add is lock-free and thread-safe
    auto index = d_ptr->currentIndex.fetch_add(1) % static_cast<int>(d_ptr->threads.size());
    d_ptr->threads[index]->handleConnection(socketDescriptor);
}

int ThreadPool::activeThreadCount() const
{ return static_cast<int>(d_ptr->threads.size()); }
