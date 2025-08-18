#include "sqlitetest.hpp"
#include "sqliteutils.hpp"

#include <QDir>
#include <QMutex>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

#include <mutex>

class SqliteTest::SqliteTestPrivate
{
public:
    explicit SqliteTestPrivate(SqliteTest *q)
        : q_ptr(q)
    {
        dataBaseConnection.dataBasePath = QString("%1/%2").arg(QDir::tempPath()).arg("test.db");

        static std::once_flag onceFlag;
        std::call_once(onceFlag, [this]() { QFile::remove(this->dataBaseConnection.dataBasePath); });

        dataBaseConnection.connectionName = getDatabaseConnectionName();

        createTable();

        qInfo() << "DataBaseTestPrivate connectionName: " << dataBaseConnection.connectionName
                << QThread::currentThread();
    }

    ~SqliteTestPrivate()
    {
        QMutexLocker locker(&mutex);
        removeDatabase(dataBaseConnection);
    }

    bool createTable()
    {
        const auto createTable
            = QString(
                  "CREATE TABLE IF NOT EXISTS [%1]("
                  " [id] INTEGER NOT NULL ON CONFLICT REPLACE UNIQUE ON CONFLICT "
                  "REPLACE COLLATE BINARY, "
                  " [brand] TEXT, "
                  " [num] TEXT, "
                  " [create_time] TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                  " [local_time] TEXT DEFAULT (strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime')), "
                  " PRIMARY KEY([id] COLLATE [BINARY] ASC) ON CONFLICT REPLACE)")
                  .arg(tableName);

        QMutexLocker locker(&mutex);
        auto db = getDatabase(dataBaseConnection);
        CHECK_DATABASE_VALIDITY(db)

        QSqlQuery query(db);
        if (!query.exec(createTable)) {
            qWarning() << query.lastError().text();
            return false;
        }
        return true;
    }

    SqliteTest *q_ptr;

    SqliteConnection dataBaseConnection;
    const QString tableName = "phone";

    static QMutex mutex;
};

QMutex SqliteTest::SqliteTestPrivate::mutex;

SqliteTest::SqliteTest(QObject *parent)
    : QObject{parent}
    , d_ptr(new SqliteTestPrivate(this))
{}

SqliteTest::~SqliteTest() {}

bool SqliteTest::insert(const QString &brand, int num)
{
    auto db = getDatabase(d_ptr->dataBaseConnection);
    CHECK_DATABASE_VALIDITY(db)
    QSqlQuery query(db);
    query.prepare(
        QString("INSERT INTO %1 (brand, num) VALUES (:brand, :num)").arg(d_ptr->tableName));
    query.bindValue(":brand", brand);
    query.bindValue(":num", num);

    QMutexLocker locker(&d_ptr->mutex);
    if (!query.exec()) {
        qCritical() << query.lastError().text();
        return false;
    }
    qInfo() << "Last inserted id:" << query.lastInsertId().toInt();
    return true;
}

bool SqliteTest::readLastRecord()
{
    auto db = getDatabase(d_ptr->dataBaseConnection);
    CHECK_DATABASE_VALIDITY(db)

    QSqlQuery query(db);
    if (!query.exec(QString("SELECT id, brand, num, create_time, local_time "
                            "FROM %1 ORDER BY id DESC LIMIT 1")
                        .arg(d_ptr->tableName))) {
        qCritical() << query.lastError().text();
        return false;
    }
    if (!query.next()) {
        qWarning() << "No records found.";
        return false;
    }
    auto text = QString(
                    "Last Record - ID: %1, Brand: %2\t, Num: %3\t, Created At: %4, Local Time: %5")
                    .arg(QString::number(query.value("id").toInt()),
                         query.value("brand").toString(),
                         QString::number(query.value("num").toInt()),
                         query.value("create_time").toString(),
                         query.value("local_time").toString());
    qDebug().noquote() << text;
    return true;
}
