#include "sqlutils.hpp"

#include <QCoreApplication>
#include <QMutex>
#include <QSqlDatabase>
#include <QWaitCondition>

#include <thread>

class CountDownLatch
{
    Q_DISABLE_COPY_MOVE(CountDownLatch)
public:
    explicit CountDownLatch(int count)
        : m_count(count)
    {}
    void countDown()
    {
        QMutexLocker lock(&m_mutex);
        if (--m_count == 0) {
            m_condition.wakeAll();
        }
    }
    int count() const
    {
        QMutexLocker lock(&m_mutex);
        return m_count;
    }

    void wait()
    {
        QMutexLocker lock(&m_mutex);
        while (m_count > 0) {
            m_condition.wait(&m_mutex);
        }
    }

private:
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    int m_count;
};

void insertThread(const QString &brand, CountDownLatch &countDown)
{
    SqlUtils sqlUtils;
    countDown.wait();
    for (int i = 0; i < 500; i++) {
        sqlUtils.insert(brand, i);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        qWarning() << "This demo needs the SQLITE driver";
        return -1;
    }

    const QStringList brands{"Apple", "Samsung", "Xiaomi", "Huawei", "Oppo", "Vivo", "Realme"};
    CountDownLatch countDown(1);
    std::vector<std::thread> threads;
    for (const auto &brand : brands) {
        threads.emplace_back(insertThread, brand, std::ref(countDown));
    }
    countDown.countDown();
    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}
