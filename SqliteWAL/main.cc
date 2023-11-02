#include "sqlutils.hpp"

#include <QCoreApplication>

#include <thread>

void insertThread(const QString &brand)
{
    SqlUtils sqlUtils;
    for (int i = 0; i < 500; i++) {
        sqlUtils.insert(brand, i);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SqlUtils sqlUtils;

    const QStringList brands{"Apple", "Samsung", "Xiaomi", "Huawei", "Oppo", "Vivo", "Realme"};
    std::vector<std::thread> threads;
    for (const auto &brand : brands) {
        threads.emplace_back(insertThread, brand);
    }
    for (auto &thread : threads) {
        thread.join();
    }

    return 0;
}
