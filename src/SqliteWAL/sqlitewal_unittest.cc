#include "sqlitetest.hpp"
#include "sqliteutils.hpp"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include <atomic>
#include <thread>
#include <vector>

using namespace Qt::StringLiterals;

// =============================================================================
// TestSqliteUtils — 测试 sqliteutils 中的工具函数
// =============================================================================
class TestSqliteUtils : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // 连接管理
    void testGetDatabaseConnectionNameUniqueness();
    void testGetDatabaseNewConnection();
    void testGetDatabaseExistingConnection();
    void testRemoveDatabase();
    void testRemoveDatabaseNonExistent();

    // checkDatabaseValidity
    void testCheckDatabaseValidityValid();
    void testCheckDatabaseValidityInvalid();

    // PRAGMA 配置
    void testWalModeEnabled();

private:
    QTemporaryDir m_tempDir;
    QString m_dbPath;
};

void TestSqliteUtils::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    m_dbPath = m_tempDir.filePath(u"test_utils.db"_s);
}

void TestSqliteUtils::cleanupTestCase()
{
    // QTemporaryDir 会在析构时自动清理
}

// --- 连接管理 ---

void TestSqliteUtils::testGetDatabaseConnectionNameUniqueness()
{
    const auto name1 = getDatabaseConnectionName();
    const auto name2 = getDatabaseConnectionName();
    const auto name3 = getDatabaseConnectionName();

    QVERIFY(name1 != name2);
    QVERIFY(name2 != name3);
    QVERIFY(name1.startsWith(u"SQLITE_CONNECTION_"_s));
}

void TestSqliteUtils::testGetDatabaseNewConnection()
{
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_dbPath;

    {
        auto db = getDatabase(conn);
        QVERIFY(checkDatabaseValidity(db));
        QVERIFY(db.isOpen());
        QCOMPARE(db.driverName(), u"QSQLITE"_s);
    }
    // db 已析构，安全移除
    removeDatabase(conn);
}

void TestSqliteUtils::testGetDatabaseExistingConnection()
{
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_dbPath;

    {
        // 第一次获取：创建新连接
        auto db1 = getDatabase(conn);
        QVERIFY(db1.isValid());
        QVERIFY(db1.isOpen());

        // 第二次获取：返回已有连接
        auto db2 = getDatabase(conn);
        QVERIFY(db2.isValid());
        QVERIFY(db2.isOpen());
        QCOMPARE(db2.connectionName(), conn.connectionName);
    }
    // db1, db2 已析构，安全移除
    removeDatabase(conn);
}

void TestSqliteUtils::testRemoveDatabase()
{
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_tempDir.filePath(u"test_remove.db"_s);

    {
        auto db = getDatabase(conn);
        QVERIFY(db.isValid());
    }
    // db 已析构，安全移除
    removeDatabase(conn);
    QVERIFY(!QSqlDatabase::contains(conn.connectionName));
}

void TestSqliteUtils::testRemoveDatabaseNonExistent()
{
    SqliteConnection conn;
    conn.connectionName = u"NONEXISTENT_CONNECTION"_s;
    conn.dataBasePath = u"/nonexistent/path.db"_s;

    // 移除不存在的连接不应崩溃
    removeDatabase(conn);
    QVERIFY(!QSqlDatabase::contains(conn.connectionName));
}

// --- checkDatabaseValidity ---

void TestSqliteUtils::testCheckDatabaseValidityValid()
{
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_tempDir.filePath(u"test_valid.db"_s);

    {
        auto db = getDatabase(conn);
        QVERIFY(checkDatabaseValidity(db));
    }
    removeDatabase(conn);
}

void TestSqliteUtils::testCheckDatabaseValidityInvalid()
{
    QSqlDatabase invalidDb;
    QVERIFY(!checkDatabaseValidity(invalidDb));
}

// --- PRAGMA 配置 ---

void TestSqliteUtils::testWalModeEnabled()
{
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_tempDir.filePath(u"test_wal.db"_s);

    {
        auto db = getDatabase(conn);
        QVERIFY(db.isValid());

        QSqlQuery query(db);
        QVERIFY(query.exec("PRAGMA journal_mode"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString().toUpper(), u"WAL"_s);
    }
    removeDatabase(conn);
}

// =============================================================================
// TestSqliteTest — 测试 SqliteTest 的 CRUD 与并发行为
// =============================================================================
class TestSqliteTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    // 基本 CRUD
    void testInsert();
    void testInsertMultipleBrands();
    void testReadLastRecord();
    void testReadLastRecordEmptyTable();
    void testInsertAndReadConsistency();
    void testInsertNumericValues();

    // 并发安全
    void testConcurrentInsert();
    void testConcurrentReadWrite();

private:
    QTemporaryDir m_tempDir;
    QString m_dbPath;
};

void TestSqliteTest::initTestCase()
{ QVERIFY(m_tempDir.isValid()); }

void TestSqliteTest::init()
{
    // 每个测试使用独立的数据库文件
    m_dbPath = m_tempDir.filePath(
        QString(u"test_%1.db"_s).arg(QString::number(reinterpret_cast<quintptr>(this), 16)));
}

void TestSqliteTest::cleanup()
{
    // SqliteTest 析构时会自动 removeDatabase
}

// --- 基本 CRUD ---

void TestSqliteTest::testInsert()
{
    SqliteTest test(m_dbPath);
    QVERIFY(test.insert(u"Apple"_s, 1));
}

void TestSqliteTest::testInsertMultipleBrands()
{
    SqliteTest test(m_dbPath);

    QVERIFY(test.insert(u"Apple"_s, 1));
    QVERIFY(test.insert(u"Samsung"_s, 2));
    QVERIFY(test.insert(u"Xiaomi"_s, 3));
}

void TestSqliteTest::testReadLastRecord()
{
    SqliteTest test(m_dbPath);
    QVERIFY(test.insert(u"Apple"_s, 42));
    QVERIFY(test.readLastRecord());
}

void TestSqliteTest::testReadLastRecordEmptyTable()
{
    // 使用独立路径确保空表
    auto emptyDbPath = m_tempDir.filePath(u"empty_table.db"_s);
    SqliteTest test(emptyDbPath);
    // 不插入任何数据，直接读取
    QVERIFY(!test.readLastRecord());
}

void TestSqliteTest::testInsertAndReadConsistency()
{
    SqliteTest test(m_dbPath);

    // 插入多条记录，验证最后一条
    QVERIFY(test.insert(u"BrandA"_s, 100));
    QVERIFY(test.insert(u"BrandB"_s, 200));
    QVERIFY(test.insert(u"BrandC"_s, 300));

    // 通过原始 SQL 验证数据一致性
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = m_dbPath;

    {
        auto db = getDatabase(conn);
        QVERIFY(db.isValid());

        QSqlQuery query(db);
        QVERIFY(query.exec("SELECT brand, num FROM phone ORDER BY id DESC LIMIT 1"));
        QVERIFY(query.next());
        QCOMPARE(query.value(u"brand"_s).toString(), u"BrandC"_s);
        QCOMPARE(query.value(u"num"_s).toInt(), 300);
    }
    removeDatabase(conn);
}

void TestSqliteTest::testInsertNumericValues()
{
    SqliteTest test(m_dbPath);

    QVERIFY(test.insert(u"Zero"_s, 0));
    QVERIFY(test.insert(u"Negative"_s, -1));
    QVERIFY(test.insert(u"Large"_s, 999999));
}

// --- 并发安全 ---

void TestSqliteTest::testConcurrentInsert()
{
    const int threadCount = 5;
    const int insertsPerThread = 100;

    auto concurrentDbPath = m_tempDir.filePath(u"concurrent_insert.db"_s);

    std::atomic<int> successCount{0};
    {
        std::vector<std::thread> threads;
        for (int t = 0; t < threadCount; ++t) {
            threads.emplace_back([&, t]() {
                SqliteTest test(concurrentDbPath);
                for (int i = 0; i < insertsPerThread; ++i) {
                    if (test.insert(QString(u"Thread%1"_s).arg(t), i)) {
                        successCount.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    QCOMPARE(successCount.load(), threadCount * insertsPerThread);

    // 验证总记录数
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = concurrentDbPath;

    {
        auto db = getDatabase(conn);
        QVERIFY(db.isValid());

        QSqlQuery query(db);
        QVERIFY(query.exec("SELECT COUNT(*) FROM phone"));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), threadCount * insertsPerThread);
    }
    removeDatabase(conn);
}

void TestSqliteTest::testConcurrentReadWrite()
{
    const int threadCount = 4;
    const int operationsPerThread = 50;

    auto rwDbPath = m_tempDir.filePath(u"concurrent_rw.db"_s);

    // 先插入初始数据
    {
        SqliteTest test(rwDbPath);
        QVERIFY(test.insert(u"Initial"_s, 0));
    }

    std::atomic<int> readSuccess{0};
    std::atomic<int> writeSuccess{0};
    {
        std::vector<std::thread> threads;
        for (int t = 0; t < threadCount; ++t) {
            threads.emplace_back([&, t]() {
                SqliteTest test(rwDbPath);
                for (int i = 0; i < operationsPerThread; ++i) {
                    if (t % 2 == 0) {
                        // 写线程
                        if (test.insert(QString(u"Writer%1"_s).arg(t), i)) {
                            writeSuccess.fetch_add(1, std::memory_order_relaxed);
                        }
                    } else {
                        // 读线程
                        if (test.readLastRecord()) {
                            readSuccess.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                }
            });
        }
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    // WAL 模式下读写不应失败
    QVERIFY2(writeSuccess.load() > 0, "At least some writes should succeed");
    QVERIFY2(readSuccess.load() > 0, "At least some reads should succeed");

    // 清理连接
    SqliteConnection conn;
    conn.connectionName = getDatabaseConnectionName();
    conn.dataBasePath = rwDbPath;
    {
        auto db = getDatabase(conn);
        Q_UNUSED(db)
    }
    removeDatabase(conn);
}

// =============================================================================
// 自定义 main：运行两个测试类
// =============================================================================
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int result = 0;

    TestSqliteUtils testUtils;
    result |= QTest::qExec(&testUtils, argc, argv);

    TestSqliteTest testTest;
    result |= QTest::qExec(&testTest, argc, argv);

    return result;
}

#include "sqlitewal_unittest.moc"
