#include "sqliteutils.hpp"

#include <QFileInfo>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include <mutex>

namespace {

void logSQLiteRuntimeInfo(const QSqlDatabase &db)
{
    static std::once_flag once;
    std::call_once(once, [&db]() {
        if (!db.isValid()) {
            qCritical() << "Invalid database connection.";
            return;
        }
        QSqlQuery query(db);
        if (query.exec("SELECT sqlite_version()") && query.next()) {
            qInfo() << "SQLite version:" << query.value(0).toString();
        } else {
            qWarning() << "Failed to get SQLite version:" << query.lastError().text();
            return;
        }
        if (query.exec("PRAGMA compile_options")) {
            qInfo() << "SQLite compile options:";
            while (query.next()) {
                qInfo() << "  -" << query.value(0).toString();
            }
        } else {
            qWarning() << "Failed to get SQLite compile options:" << query.lastError().text();
            return;
        }

        auto *driver = db.driver();
        qInfo().noquote().nospace()
            << "SQLite driver capabilities:\n"
            << "  Transactions        : " << driver->hasFeature(QSqlDriver::Transactions) << '\n'
            << "  LastInsertId        : " << driver->hasFeature(QSqlDriver::LastInsertId) << '\n'
            << "  BatchOperations     : " << driver->hasFeature(QSqlDriver::BatchOperations) << '\n'
            << "  SimpleLocking       : " << driver->hasFeature(QSqlDriver::SimpleLocking) << '\n'
            << "  QuerySize           : " << driver->hasFeature(QSqlDriver::QuerySize) << '\n'
            << "  BLOB                : " << driver->hasFeature(QSqlDriver::BLOB) << '\n'
            << "  Unicode             : " << driver->hasFeature(QSqlDriver::Unicode) << '\n'
            << "  PreparedQueries     : " << driver->hasFeature(QSqlDriver::PreparedQueries) << '\n'
            << "  PositionalPlaceholders : "
            << driver->hasFeature(QSqlDriver::PositionalPlaceholders) << '\n'
            << "  NamedPlaceholders   : " << driver->hasFeature(QSqlDriver::NamedPlaceholders)
            << '\n'
            << "  MultipleResultSets  : " << driver->hasFeature(QSqlDriver::MultipleResultSets)
            << '\n'
            << "  CancelQuery         : " << driver->hasFeature(QSqlDriver::CancelQuery) << '\n'
            << "  LowPrecisionNumbers : " << driver->hasFeature(QSqlDriver::LowPrecisionNumbers);
    });
}

void executeOptions(QSqlQuery &query, const QStringList &options)
{
    for (const auto &option : std::as_const(options)) {
        if (!query.exec(option)) {
            qWarning() << "Failed to execute " << option << ":" << query.lastError().text();
        }
    }
}

QString canonicalPath(const QString &path)
{
    return QFileInfo(path).canonicalFilePath();
}

namespace {

static QMap<QString, int> connectionCount;

}

void incConnectionCount(const QString &dataBasePath)
{
    auto path = canonicalPath(dataBasePath);
    if (connectionCount.contains(path)) {
        connectionCount[path]++;
    } else {
        connectionCount[path] = 1;
    }
}

bool decConnectionCount(const QString &dataBasePath)
{
    auto path = canonicalPath(dataBasePath);
    if (connectionCount.contains(path)) {
        if (--connectionCount[path] == 0) {
            connectionCount.remove(path);
            return true; // 连接计数归零，表示可以删除
        }
    }
    return false;
}

const QStringList kPragmaOptions = {
    "PRAGMA journal_mode = WAL;",          // 启用WAL模式
    "PRAGMA optimize = 0x10002;",          // 启用自动索引和查询优化
    "PRAGMA auto_vacuum = INCREMENTAL;",   // 启用自动VACUUM
    "PRAGMA default_temp_store = MEMORY;", // 启用内存临时表
    "PRAGMA temp_store = MEMORY;",         // 启用内存临时表
    "PRAGMA page_size = 4096;",            // 设置页面大小
    "PRAGMA cache_size = -32768;",         // 设置缓存大小，单位为KB
    "PRAGMA wal_autocheckpoint = 1000;",   // 设置WAL自动检查点间隔
    "PRAGMA mmap_size = 268435456;",       // 启用内存映射，最大256MB
    "PRAGMA cache_spill = false;"          // 禁用缓存溢出
    // "PRAGMA synchronous = NORMAL;",        // 性能提升，耐久性略降
    // "PRAGMA locking_mode = EXCLUSIVE;",    // 独占锁，适合单进程
    // "PRAGMA foreign_keys = ON;",           // 启用外键约束
};

const QStringList kRemoveOptions = {
    "ANALYZE;",                             // 分析数据库
    "PRAGMA optimize;",                     // 优化数据库
    "PRAGMA incremental_vacuum;",           // 增量VACUUM
    "PRAGMA journal_size_limit = 8388608;", // 限制日志文件大小为8MB
    "PRAGMA wal_checkpoint(TRUNCATE);",     // 检查点并截断WAL文件
    "PRAGMA shrink_memory;",                // 释放内存
};

} // namespace

QSqlDatabase getDatabase(const SqliteConnection &dataBaseConnection)
{
    auto db = QSqlDatabase::database(dataBaseConnection.connectionName);
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", dataBaseConnection.connectionName);
        db.setDatabaseName(dataBaseConnection.dataBasePath);
        incConnectionCount(dataBaseConnection.dataBasePath);
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            qCritical() << "Failed to open database:" << db.lastError().text();
            // 尝试删除数据库文件并重新创建
            QFile(dataBaseConnection.dataBasePath).remove();
            db.open();
        }
        QSqlQuery query(db);
        executeOptions(query, kPragmaOptions);

        logSQLiteRuntimeInfo(db);
    }

    return db;
}

void removeDatabase(const SqliteConnection &dataBaseConnection)
{
    auto connectionName = dataBaseConnection.connectionName;
    if (!QSqlDatabase::contains(connectionName)) {
        return;
    }

    if (decConnectionCount(dataBaseConnection.dataBasePath)) {
        QSqlQuery query(getDatabase(dataBaseConnection));
        executeOptions(query, kRemoveOptions);
    }

    QSqlDatabase::removeDatabase(connectionName);
}

QString getDatabaseConnectionName()
{
    static std::atomic_llong id = 1;
    return QString("SQLITE_CONNECTION_%1").arg(id.fetch_add(1));
}
