#include "databasetest.hpp"

#include <QCoreApplication>
#include <QSqlDatabase>

#include <thread>

void insertThread(const QString &brand)
{
    DataBaseTest test;
    for (int i = 0; i < 1000; i++) {
        test.insert(brand, i);
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
    std::vector<std::thread> threads;
    for (const auto &brand : std::as_const(brands)) {
        threads.emplace_back(insertThread, brand);
    }
    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}
