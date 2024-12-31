#pragma once

#include <QSqlDatabase>

struct DataBaseConnection
{
    QString connectionName;
    QString dataBasePath;
};

QSqlDatabase getDatabase(const DataBaseConnection &dataBaseConnection);

void removeDatabase(const QString &connectionName);

QString getDatabaseConnectionName();

#define CHECK_DATABASE_VALIDITY(database) \
    if (!(database).isValid()) { \
        qWarning() << (database).lastError().text(); \
        return false; \
    }
