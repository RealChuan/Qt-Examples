#pragma once

#include <QSqlDatabase>

struct SqliteConnection
{
    QString connectionName;
    QString dataBasePath;
};

/*
 * 使用新的connectionName时，第一次调用时，必须保证线程安全
 */
QSqlDatabase getDatabase(const SqliteConnection &dataBaseConnection);

/*
 * 调用时，必须保证线程安全
 */
void removeDatabase(const SqliteConnection &dataBaseConnection);

QString getDatabaseConnectionName();

#define CHECK_DATABASE_VALIDITY(database) \
    if (!(database).isValid()) { \
        qWarning() << (database).lastError().text(); \
        return false; \
    }
