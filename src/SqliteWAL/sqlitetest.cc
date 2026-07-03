#include "sqlitetest.hpp"
#include "sqliteutils.hpp"

#include <QDir>
#include <QMutex>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>

using namespace Qt::StringLiterals;

class SqliteTest::SqliteTestPrivate
{
public:
    explicit SqliteTestPrivate(SqliteTest *q, const QString &dbPath) : q_ptr(q)
    {
        dataBaseConnection.dataBasePath
            = dbPath.isEmpty() ? QDir::tempPath() + u"/test.db"_s : dbPath;

        static std::once_flag onceFlag;
        std::call_once(onceFlag,
                       [this]() { QFile::remove(this->dataBaseConnection.dataBasePath); });

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
                  " [num] INTEGER, "
                  " [create_time] TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                  " [local_time] TEXT DEFAULT (strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime')), "
                  " PRIMARY KEY([id] COLLATE [BINARY] ASC) ON CONFLICT REPLACE)")
                  .arg(tableName);

        QMutexLocker locker(&mutex);
        auto db = getDatabase(dataBaseConnection);
        if (!checkDatabaseValidity(db))
            return false;

        QSqlQuery query(db);
        if (!query.exec(createTable)) {
            qWarning() << query.lastError().text();
            return false;
        }
        return true;
    }

    SqliteTest *q_ptr;

    SqliteConnection dataBaseConnection;
    const QString tableName = u"phone"_s;

    static QMutex mutex;
};

QMutex SqliteTest::SqliteTestPrivate::mutex;

SqliteTest::SqliteTest(const QString &dbPath, QObject *parent)
    : QObject{parent}, d_ptr(std::make_unique<SqliteTestPrivate>(this, dbPath))
{}

SqliteTest::~SqliteTest() = default;

bool SqliteTest::insert(const QString &brand, int num)
{
    auto db = getDatabase(d_ptr->dataBaseConnection);
    if (!checkDatabaseValidity(db))
        return false;

    QSqlQuery query(db);
    query.prepare(
        QString("INSERT INTO %1 (brand, num) VALUES (:brand, :num)").arg(d_ptr->tableName));
    query.bindValue(u":brand"_s, brand);
    query.bindValue(u":num"_s, num);

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
    if (!checkDatabaseValidity(db))
        return false;

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
    auto text
        = QString("Last Record - ID: %1, Brand: %2\t, Num: %3\t, Created At: %4, Local Time: %5")
              .arg(QString::number(query.value(u"id"_s).toInt()),
                   query.value(u"brand"_s).toString(),
                   QString::number(query.value(u"num"_s).toInt()),
                   query.value(u"create_time"_s).toString(),
                   query.value(u"local_time"_s).toString());
    qDebug().noquote() << text;
    return true;
}
