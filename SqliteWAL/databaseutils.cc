#include "databaseutils.hpp"

#include <QFile>
#include <QSqlQuery>

QSqlDatabase getDatabase(const DataBaseConnection &dataBaseConnection)
{
    auto db = QSqlDatabase::database(dataBaseConnection.connectionName);
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", dataBaseConnection.connectionName);
        db.setDatabaseName(dataBaseConnection.dataBasePath);
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            QFile(dataBaseConnection.dataBasePath).remove();
            db.open();
        }
        QSqlQuery walQuery(db);
        walQuery.exec("PRAGMA journal_mode = WAL;");
    }

    return db;
}

void removeDatabase(const QString &connectionName)
{
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

QString getDatabaseConnectionName()
{
    static std::atomic_llong id = 1;
    return QString("connection_%1").arg(id.fetch_add(1, std::memory_order_seq_cst));
}
