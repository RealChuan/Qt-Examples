#include "databasetest.hpp"
#include "databaseutils.hpp"

#include <QDir>
#include <QMutex>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

class DataBaseTest::DataBaseTestPrivate
{
public:
    explicit DataBaseTestPrivate(DataBaseTest *q)
        : q_ptr(q)
    {
        dataBaseConnection.dataBasePath = QString("%1/%2").arg(QDir::tempPath()).arg("test.db");
        dataBaseConnection.connectionName = getDatabaseConnectionName();

        createTable();

        qInfo() << "DataBaseTestPrivate connectionName: " << dataBaseConnection.connectionName
                << QThread::currentThread();
    }

    ~DataBaseTestPrivate() { removeDatabase(dataBaseConnection.connectionName); }

    bool createTable()
    {
        const auto createTable
            = QString("CREATE TABLE [%1]("
                      " [id] INTEGER NOT NULL ON CONFLICT REPLACE UNIQUE ON CONFLICT "
                      "REPLACE COLLATE BINARY, "
                      " [brand] TEXT, "
                      " [num] TEXT, "
                      " [create_time] TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                      " PRIMARY KEY([id] COLLATE [BINARY] ASC) ON CONFLICT REPLACE)")
                  .arg(tableName);

        QMutexLocker locker(&mutex);
        auto db = getDatabase(dataBaseConnection);
        CHECK_DATABASE_VALIDITY(db)

        auto *driver = db.driver();
        qInfo() << "Support Transactions: " << driver->hasFeature(QSqlDriver::Transactions)
                << "Support LastInsertId: " << driver->hasFeature(QSqlDriver::LastInsertId)
                << "Support BatchOperations: " << driver->hasFeature(QSqlDriver::BatchOperations)
                << "Support SimpleLocking: " << driver->hasFeature(QSqlDriver::SimpleLocking);

        if (db.tables().contains(tableName)) {
            return true;
        }

        QSqlQuery query(db);
        if (!query.exec(createTable)) {
            qWarning() << query.lastError().text();
            return false;
        }
        return true;
    }

    DataBaseTest *q_ptr;

    DataBaseConnection dataBaseConnection;
    const QString tableName = "phone";

    static QMutex mutex;
};

QMutex DataBaseTest::DataBaseTestPrivate::mutex;

DataBaseTest::DataBaseTest(QObject *parent)
    : QObject{parent}
    , d_ptr(new DataBaseTestPrivate(this))
{}

DataBaseTest::~DataBaseTest() {}

bool DataBaseTest::insert(const QString &brand, int num)
{
    QMutexLocker locker(&d_ptr->mutex);
    auto db = getDatabase(d_ptr->dataBaseConnection);
    CHECK_DATABASE_VALIDITY(db)
    QSqlQuery query(db);
    query.prepare(
        QString("INSERT INTO %1 (brand, num) VALUES (:brand, :num)").arg(d_ptr->tableName));
    query.bindValue(":brand", brand);
    query.bindValue(":num", num);
    if (!query.exec()) {
        qCritical() << query.lastError().text();
        return false;
    }
    qInfo() << "Last inserted id:" << query.lastInsertId().toInt();
    return true;
}
