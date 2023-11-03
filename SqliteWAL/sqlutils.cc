#include "sqlutils.hpp"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class SqlUtils::SqlUtilsPrivate
{
public:
    SqlUtilsPrivate(SqlUtils *q)
        : q_ptr(q)
    {
        openSqliteDatabase();
    }

    ~SqlUtilsPrivate() { QSqlDatabase::removeDatabase(connectionName); }

    void openSqliteDatabase()
    {
        s_instanceCount.ref();
        connectionName = QString("connection%1").arg(s_instanceCount.loadRelaxed());
        // qDebug() << connectionName;
        auto db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(databaseName);
        if (!db.open()) {
            qCritical() << db.lastError().text();
            return;
        }
        // Enable WAL mode
        QSqlQuery walQuery(db);
        walQuery.exec("PRAGMA journal_mode = WAL;");

        auto tables = db.tables();
        if (tables.contains(tableName, Qt::CaseInsensitive)) {
            return;
        }
        const auto createTableSql = QString("CREATE TABLE IF NOT EXISTS %1 ("
                                            "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                                            "brand TEXT, "
                                            "num INTEGER) ")
                                        .arg(tableName);
        QSqlQuery query(db);
        if (!query.exec(createTableSql)) {
            qCritical() << query.lastError().text();
            db.close();
        }
    }

    void insert(const QString &brand, int num)
    {
        auto db = QSqlDatabase::database(connectionName);
        if (!db.isValid() || !db.isOpen()) {
            return;
        }
        QSqlQuery query(db);
        query.prepare(QString("INSERT INTO %1 (brand, num) VALUES (:brand, :num)").arg(tableName));
        query.bindValue(":brand", brand);
        query.bindValue(":num", num);
        if (!query.exec()) {
            qCritical() << query.lastError().text();
        }
    }

    SqlUtils *q_ptr;

    QString connectionName;
    const QString databaseName = "example.db";
    const QString tableName = "phone";

    static QAtomicInt s_instanceCount;
};

QAtomicInt SqlUtils::SqlUtilsPrivate::s_instanceCount = 0;

SqlUtils::SqlUtils(QObject *parent)
    : QObject(parent)
    , d_ptr(new SqlUtilsPrivate(this))
{}

SqlUtils::~SqlUtils() {}

void SqlUtils::insert(const QString &brand, int num)
{
    d_ptr->insert(brand, num);
}
