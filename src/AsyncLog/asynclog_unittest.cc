// AsyncLog 用法示例：本单测即 demo，集成方式参考测试代码。
//
//     #include "asynclog.hpp"
//
//     int main(int argc, char *argv[])
//     {
//         QCoreApplication app(argc, argv);
//         auto *logger = AsyncLog::Logger::instance();
//         logger->start({.logPath = "logs", .console = true, .file = true});
//
//         qInfo() << "hello AsyncLog";      // 任意线程直接用 qDebug/qInfo/qWarning...
//         // 自定义分类（Q_LOGGING_CATEGORY）经 qC 宏携带，行内显示为 [my.cat]：
//         // qCInfo(lcMyCat) << "categorized hello";
//         // UI 类消费者可连接批量信号做实时日志视图：
//         // connect(logger, &AsyncLog::Logger::batchReady, view, &LogView::appendLines);
//
//         logger->shutdown();            // main 返回前调用，保证日志落盘
//         return 0;
//     }

#include <QtTest/QtTest>

#include "asynclog.hpp"
#include "logbuffer.hpp"
#include "logsink.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QSet>
#include <QSignalSpy>
#include <QSysInfo>
#include <QTemporaryDir>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <span>
#include <string>
#include <thread>
#include <vector>

using namespace AsyncLog;

// 演示用日志分类：qC 宏携带 category，行内槽位显示 [asynclog.demo] 而非 [default]。
// 演示行用 qCWarning——自定义分类的 Debug/Info 在 release 构建默认被 Qt 过滤，
// warning 及以上恒有效，保证断言与构建类型无关
Q_LOGGING_CATEGORY(lcAsyncLogDemo, "asynclog.demo")

namespace {

// 读取目录下所有 *.log 内容，按行返回（忽略空行）
[[nodiscard]] QStringList readAllLines(const QString &dir)
{
    QStringList lines;
    const QDir logDir(dir);
    const auto files = logDir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot);
    for (const auto &info : files) {
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const auto content = QString::fromUtf8(file.readAll());
        for (const auto &line : content.split('\n')) {
            if (!line.isEmpty()) {
                lines.append(line);
            }
        }
    }
    return lines;
}

// 预置一个过期文件（mtime 前拨），用于清理测试
void makeStaleFile(const QString &path, const QByteArray &content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(content);
    QVERIFY(file.setFileTime(QDateTime::currentDateTime().addDays(-30),
                             QFileDevice::FileModificationTime));
    file.close();
}

// 严格命名模式：版本槽位按期望原文精确匹配，主机/进程取当前
// 进程实际值，时间戳部分通配。锚定整名，多一个或少一个槽位都不匹配
[[nodiscard]] QRegularExpression expectedNamePattern(const QString &expectedVersion)
{
    const QString versionSlot = QStringLiteral("_") + QRegularExpression::escape(expectedVersion);
    return QRegularExpression(QRegularExpression::anchoredPattern(
        QString("%1%2_%3_%4_\\d{8}_\\d{6}(_\\d+)?\\.log")
            .arg(QRegularExpression::escape(QCoreApplication::applicationName()),
                 versionSlot,
                 QRegularExpression::escape(QSysInfo::machineHostName()),
                 QString::number(QCoreApplication::applicationPid()))));
}

} // namespace

class AsyncLogUnitTest : public QObject
{
    Q_OBJECT

private slots:

    void init() { m_dir = std::make_unique<QTemporaryDir>(); }

    void cleanup()
    {
        Logger::instance()->shutdown();
        m_dir.reset();
    }

    // 常规用法冒烟测试：启动 → 混合级别输出 → 关停，打印完整日志行
    void testNormalUsage();

    // Logger::start/shutdown 幂等，可循环使用
    void testLifecycle();

    // 级别过滤基于 severity()：QtInfoMsg(4) 数值高于 QtWarningMsg(1)，
    // 按数值比较会把 warning 丢掉
    void testLevelFilter();

    // 行格式与单文件命名
    void testFileOutput();

    // 超过 maxFileSize 触发滚动，数据一条不丢（同秒滚动加序号）
    void testSizeRoll();

    // 文件名版本槽位：<base>_<version>_<host>_<pid>_<时间>，版本取 applicationVersion
    void testFileNameVersionSlot();

    // 版本号清洗：Windows 非法字符与空白替换为 '-'
    void testFileNameVersionSanitized();

    // 未设置版本号时版本槽位填 unknown 占位
    void testFileNameUnknownVersion();

    // 过期清理只删匹配自身命名模式的文件
    void testAutoDeletePattern();

    // 运行期间清空全部历史日志（正在写入的当前文件除外）
    void testPurgeLogFiles();

    // batchReady 信号：后端批量送达，无人连接时跳过构建
    void testBatchReadySignal();

    // LogBuffer 多生产单消费：8 线程并发无丢失
    void testBufferMpsc();

    // 分层饱和：高水位后 DEBUG/INFO 丢弃计数，错误级无条件收下
    void testBufferSaturation();

    // shutdown 必须排空队列
    void testShutdownDrains();

    // QtFatalMsg 同步落盘（处理器返回后 Qt 才 abort）
    void testFatalFlushes();

    // ConsoleSink 行截断
    void testConsoleTruncate();

    // 吞吐演示：4 线程 × 25000 条
    void testThroughput();

    // LogBuffer push/pop 微基准
    void testLogBufferBenchmark();

private:
    std::unique_ptr<QTemporaryDir> m_dir;
};

void AsyncLogUnitTest::testNormalUsage()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = true, .file = true, .level = QtDebugMsg});

    qDebug() << "normal usage debug";
    qInfo() << "server started, port 8080";
    qWarning() << "config missing, using defaults";
    qCritical() << "database connection failed";
    qCWarning(lcAsyncLogDemo) << "categorized warning from asynclog.demo";

    logger->shutdown();

    const auto lines = readAllLines(m_dir->path());
    QCOMPARE(lines.size(), qsizetype(5)); // 四个级别各一行 + 一条自定义分类

    // 把完整日志行打印出来（此时已 shutdown，走 Qt 默认处理器）
    for (const auto &line : lines) {
        qWarning().noquote() << line;
    }

    // 行结构逐项校验（category 槽位为消息所属分类）：
    // 2026-09-27 14:30:05.123 [I] [t 1a2b] [default] server started, port 8080 (asynclog_unittest.cc:148)
    // 2026-09-27 14:30:05.124 [W] [t 1a2b] [asynclog.demo] categorized warning ...
    const QRegularExpression fullLine(
        R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} \[[DIWC]\] \[t [0-9a-fA-F]+\] )"
        R"(\[[^\]]+\] .+)");
    const auto text = lines.join(QLatin1Char('\n'));
    for (const auto &line : lines) {
        QVERIFY2(fullLine.match(line).hasMatch(), qPrintable(line));
    }
    QVERIFY(text.contains("[D]"));
    QVERIFY(text.contains("[I]"));
    QVERIFY(text.contains("[W]"));
    QVERIFY(text.contains("[C]"));
    QVERIFY(text.contains("server started, port 8080"));
    QVERIFY(text.contains("[default]"));       // 裸 qInfo/qWarning 属默认分类
    QVERIFY(text.contains("[asynclog.demo]")); // qC 宏携带自定义分类

#ifndef QT_NO_DEBUG
    // debug 构建默认带 QT_MESSAGELOGCONTEXT：行尾应有位置后缀
    QVERIFY(text.contains(".cc:"));
#endif
}

void AsyncLogUnitTest::testLifecycle()
{
    const Config config{.logPath = m_dir->path(), .console = false, .file = true};
    auto *logger = Logger::instance();

    logger->start(config);
    QVERIFY(logger->isRunning());
    logger->start(config); // 幂等：重复启动被忽略
    QVERIFY(logger->isRunning());

    logger->shutdown();
    QVERIFY(!logger->isRunning());
    logger->shutdown(); // 幂等
    QVERIFY(!logger->isRunning());

    logger->start(config); // shutdown 后可再次启动
    qInfo() << "RESTART-OK";
    logger->shutdown();
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("RESTART-OK"));
}

void AsyncLogUnitTest::testLevelFilter()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});

    logger->setLevel(QtInfoMsg);
    qDebug() << "FILTER-DEBUG-NO";
    qInfo() << "FILTER-INFO-YES";
    qWarning() << "FILTER-WARNING-YES";

    logger->setLevel(QtWarningMsg);
    qInfo() << "FILTER-INFO-NO";
    qWarning() << "FILTER-WARNING-YES-2";

    logger->shutdown();

    const auto text = readAllLines(m_dir->path()).join(QLatin1Char('\n'));
    QVERIFY(text.contains("FILTER-INFO-YES"));
    QVERIFY(text.contains("FILTER-WARNING-YES"));
    QVERIFY(text.contains("FILTER-WARNING-YES-2"));
    QVERIFY(!text.contains("FILTER-DEBUG-NO"));
    QVERIFY(!text.contains("FILTER-INFO-NO"));
}

void AsyncLogUnitTest::testFileOutput()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtDebugMsg});

    qInfo() << "ASCII-LINE";
    qWarning() << "hello, world 123";

    logger->shutdown();

    const auto lines = readAllLines(m_dir->path());
    const QRegularExpression pattern(
        R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3} \[I\] \[t [0-9a-fA-F]+\] \[default\] )"
        R"(ASCII-LINE)");
    const bool matched = std::any_of(lines.cbegin(), lines.cend(), [&pattern](const auto &line) {
        return pattern.match(line).hasMatch();
    });
    QVERIFY(matched);
    QVERIFY(lines.join(QLatin1Char('\n')).contains("hello, world 123"));

    QCOMPARE(QDir(m_dir->path()).entryInfoList({"*.log"}, QDir::Files).size(), 1);
}

void AsyncLogUnitTest::testSizeRoll()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(),
                   .console = false,
                   .file = true,
                   .level = QtInfoMsg,
                   .maxFileSize = 2048,
                   .keepDays = 0});

    for (int i = 0; i < 80; ++i) {
        qInfo() << QString("size-roll-%1").arg(i, 4, 10, QLatin1Char('0'));
    }
    logger->shutdown();

    const QDir dir(m_dir->path());
    const auto files = dir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot);
    QVERIFY(files.size() >= 2); // 超阈值滚动（批级别近似，单文件可略超）

    // 命名：<base>_<version>_<host>_<pid>_<yyyyMMdd_hhmmss>[_<seq>].log，版本/主机/进程槽位通配
    const auto base = QCoreApplication::applicationName();
    const QRegularExpression namePattern(QRegularExpression::anchoredPattern(
        QString("%1_.+_\\d{8}_\\d{6}(_\\d+)?\\.log").arg(QRegularExpression::escape(base))));
    for (const auto &info : files) {
        QVERIFY(namePattern.match(info.fileName()).hasMatch());
    }

    QCOMPARE(readAllLines(m_dir->path()).size(), qsizetype(80)); // 滚动不丢数据
}

void AsyncLogUnitTest::testFileNameVersionSlot()
{
    QCoreApplication::setApplicationVersion(QStringLiteral("1.2.3"));

    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});
    qInfo() << "VERSION-SLOT-OK";
    logger->shutdown();

    const QDir dir(m_dir->path());
    const auto files = dir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot);
    QCOMPARE(files.size(), 1);
    const auto name = files.first().fileName();
    QVERIFY(expectedNamePattern(QStringLiteral("1.2.3")).match(name).hasMatch());
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("VERSION-SLOT-OK"));
}

void AsyncLogUnitTest::testFileNameVersionSanitized()
{
    // 空格与 ':' 都在清洗范围（':' 在 Windows 文件名非法）
    QCoreApplication::setApplicationVersion(QStringLiteral("1.2 beta:2"));

    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});
    qInfo() << "VERSION-SANITIZED-OK";
    logger->shutdown();

    const QDir dir(m_dir->path());
    const auto files = dir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot);
    QCOMPARE(files.size(), 1);
    const auto name = files.first().fileName();
    QVERIFY(expectedNamePattern(QStringLiteral("1.2-beta-2")).match(name).hasMatch());
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("VERSION-SANITIZED-OK"));
}

void AsyncLogUnitTest::testFileNameUnknownVersion()
{
    QCoreApplication::setApplicationVersion(QString());

    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});
    qInfo() << "UNKNOWN-VERSION-OK";
    logger->shutdown();

    const QDir dir(m_dir->path());
    const auto files = dir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot);
    QCOMPARE(files.size(), 1);
    const auto name = files.first().fileName();
    QVERIFY(expectedNamePattern(QStringLiteral("unknown")).match(name).hasMatch());
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("UNKNOWN-VERSION-OK"));
}

void AsyncLogUnitTest::testAutoDeletePattern()
{
    const QDir dir(m_dir->path());
    const auto base = QCoreApplication::applicationName();

    // 无关的过期文件：清理绝不波及
    const auto innocentPath = dir.filePath("important_data.txt");
    makeStaleFile(innocentPath, "keep me");

    // 匹配自身命名模式的过期日志（模拟另一台主机/另一进程的历史日志）：必须清理
    const auto stalePath = dir.filePath(QString("%1_myhost_123_20200101_000000.log").arg(base));
    makeStaleFile(stalePath, "old log");

    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(),
                   .console = false,
                   .file = true,
                   .level = QtInfoMsg,
                   .keepDays = 7});
    qInfo() << "auto-delete-ok";
    logger->shutdown();

    QVERIFY(QFile::exists(innocentPath)); // 只清自己的
    QVERIFY(!QFile::exists(stalePath));
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("auto-delete-ok"));
}

void AsyncLogUnitTest::testPurgeLogFiles()
{
    const QDir dir(m_dir->path());
    const auto base = QCoreApplication::applicationName();

    // 无关的文件：绝不波及
    const auto innocentPath = dir.filePath("important_data.txt");
    makeStaleFile(innocentPath, "keep me");
    // 其他进程实例留下的历史日志：同样匹配自身命名模式，应被清理
    const auto otherInstancePath
        = dir.filePath(QString("%1_remotehost_999_20200101_000000.log").arg(base));
    makeStaleFile(otherInstancePath, "other instance");

    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(),
                   .console = false,
                   .file = true,
                   .level = QtInfoMsg,
                   .maxFileSize = 2048,
                   .keepDays = 0});

    for (int i = 0; i < 80; ++i) {
        qInfo() << "purge-roll-" << i;
    }

    const auto countLogs = [&dir] {
        return dir.entryInfoList({"*.log"}, QDir::Files | QDir::NoDotAndDotDot).size();
    };
    // 等后端把 80 条全部落盘（80 条远超 2048 必然滚动出多个文件）
    QTRY_VERIFY([](const QDir &d) {
        qsizetype written = 0;
        for (const auto &line : readAllLines(d.path())) {
            if (line.contains("purge-roll-")) {
                ++written;
            }
        }
        return written;
    }(dir) >= qsizetype(80));

    const auto before = countLogs(); // 含 otherInstance，不含 innocent
    QVERIFY(before >= 2);

    // 运行期间调用：历史日志全删，正在写入的当前文件除外
    const auto removed = logger->purgeLogFiles();
    QCOMPARE(removed, before - 1);
    QCOMPARE(countLogs(), 1); // 只剩当前文件

    QVERIFY(QFile::exists(innocentPath));
    QVERIFY(!QFile::exists(otherInstancePath));

    // 当前文件继续可写
    qInfo() << "STILL-WRITING";
    logger->shutdown();
    QVERIFY(readAllLines(m_dir->path()).join(QLatin1Char('\n')).contains("STILL-WRITING"));
}

void AsyncLogUnitTest::testBatchReadySignal()
{
    auto *logger = Logger::instance();
    QSignalSpy spy(logger, &Logger::batchReady);

    // 不开任何 sink，只验证信号通路
    logger->start({.logPath = m_dir->path(), .console = false, .file = false, .level = QtInfoMsg});
    for (int i = 0; i < 10; ++i) {
        qInfo() << "SIGNAL-LINE-" << i;
    }
    logger->shutdown(); // 后端排空，批次信号已投递（队列或直连均可）

    // 信号在后端线程 emit，跨线程接收需要事件循环送达；QTRY 自带等待循环。
    // 若 QSignalSpy 走直连，shutdown 的 join 已保证写入可见，QTRY 立即通过
    QTRY_VERIFY(spy.count() >= 1);

    qsizetype total = 0;
    for (const auto &args : spy) {
        total += args.at(0).toStringList().size();
    }
    QCOMPARE(total, qsizetype(10)); // 10 条全部送达，无重复无丢失
}

void AsyncLogUnitTest::testBufferMpsc()
{
    constexpr int kThreads = 8;
    constexpr int kPerThread = 10000;

    LogBuffer buffer;
    std::vector<std::thread> producers;
    for (int t = 0; t < kThreads; ++t) {
        producers.emplace_back([&buffer] {
            for (int i = 0; i < kPerThread; ++i) {
                buffer.push(LogRecord{.type = QtDebugMsg, .message = QString("mpsc-%1").arg(i)});
            }
        });
    }
    for (auto &producer : producers) {
        producer.join();
    }

    std::size_t received = 0;
    std::vector<LogRecord> batch;
    while (buffer.popBatch(batch, std::chrono::milliseconds{100})) {
        received += batch.size();
        batch.clear();
        if (received == std::size_t(kThreads * kPerThread)) {
            break;
        }
    }
    buffer.close();
    QVERIFY(!buffer.popBatch(batch, std::chrono::milliseconds{50})); // close 后排空即终止

    QCOMPARE(received, std::size_t(kThreads * kPerThread));          // 8 万条零丢失
    QCOMPARE(buffer.takeDropped(), std::size_t{0});
}

void AsyncLogUnitTest::testBufferSaturation()
{
    constexpr int kDebugPush = 2000;
    constexpr int kCriticalPush = 100;
    const auto bigMessage = QString(200, 'x');

    // 高水位 128KB，截止期 100ms（默认 1s 会拖慢测试）
    LogBuffer buffer(128 * 1024, std::chrono::milliseconds{100});

    // 无消费者，先灌 DEBUG 至溢出：首次触顶经历一个截止期，降级后低级别直接丢弃
    for (int i = 0; i < kDebugPush; ++i) {
        buffer.push(LogRecord{.type = QtDebugMsg, .message = bigMessage});
    }
    // 降级期内错误级无条件收下（诊断价值最高，绝不丢）
    for (int i = 0; i < kCriticalPush; ++i) {
        buffer.push(LogRecord{.type = QtCriticalMsg, .message = bigMessage});
    }

    buffer.close();
    std::size_t criticals = 0;
    std::size_t debugs = 0;
    std::vector<LogRecord> batch;
    while (buffer.popBatch(batch, std::chrono::milliseconds{50})) {
        for (const auto &record : batch) {
            if (record.type == QtCriticalMsg) {
                ++criticals;
            } else {
                ++debugs;
            }
        }
        batch.clear();
    }

    QCOMPARE(criticals, std::size_t(kCriticalPush)); // 错误级全收
    QVERIFY(debugs < std::size_t(kDebugPush));       // 低级别被高水位截流
    QCOMPARE(buffer.takeDropped(),
             std::size_t(kDebugPush) - debugs);      // 丢弃计数精确闭环
}

void AsyncLogUnitTest::testShutdownDrains()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});

    constexpr int kThreads = 4;
    constexpr int kPerThread = 2500;
    std::vector<std::thread> writers;
    for (int t = 0; t < kThreads; ++t) {
        writers.emplace_back([t] {
            for (int i = 0; i < kPerThread; ++i) {
                qInfo() << QString("drain-%1-%2").arg(t).arg(i);
            }
        });
    }
    for (auto &writer : writers) {
        writer.join();
    }

    // shutdown 必须排空队列后返回
    logger->shutdown();

    const auto lines = readAllLines(m_dir->path());
    QCOMPARE(lines.size(), qsizetype(kThreads * kPerThread));

    // QDebug 对 QString 默认加引号（char 字面量不加），消息实际是 "drain-0-123"
    const QRegularExpression messagePattern(R"(\[default\] "?(drain-\d+-\d+))");
    QSet<QString> unique;
    for (const auto &line : lines) {
        const auto match = messagePattern.match(line);
        QVERIFY2(match.hasMatch(), qPrintable(line));
        unique.insert(match.captured(1));
    }
    QCOMPARE(unique.size(), 10000); // 无重复无丢失
}

void AsyncLogUnitTest::testFatalFlushes()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtDebugMsg});

    // qFatal 会 abort 进程，无法在测试中触发；直接调用处理器覆盖 fatal 同步路径。
    // 处理器返回后由 Qt 负责终止进程，测试不受影响
    const QMessageLogContext context(__FILE__, __LINE__, Q_FUNC_INFO, "default");
    messageHandler(QtFatalMsg, context, "FATAL-BEACON-42");

    logger->shutdown();

    const auto text = readAllLines(m_dir->path()).join(QLatin1Char('\n'));
    QVERIFY(text.contains("FATAL-BEACON-42"));
    QVERIFY(text.contains("[F]"));
}

void AsyncLogUnitTest::testConsoleTruncate()
{
    std::FILE *captured = std::tmpfile();
    QVERIFY(captured != nullptr);

    ConsoleSink sink(64, captured, captured);
    std::string line(1000, 'x');
    line.back() = '\n';
    sink.write(std::span<const char>{line.data(), line.size()}, false);
    sink.flush();

    std::rewind(captured);
    std::string content(128, '\0');
    const auto read = std::fread(content.data(), 1, content.size(), captured);
    QCOMPARE(read, std::size_t{65}); // 64 字节截断 + 补写的 '\n'
    QCOMPARE(content[64], '\n');
    std::fclose(captured);
}

void AsyncLogUnitTest::testThroughput()
{
    auto *logger = Logger::instance();
    logger->start({.logPath = m_dir->path(), .console = false, .file = true, .level = QtInfoMsg});

    constexpr int kThreads = 4;
    constexpr int kPerThread = 25000;
    QElapsedTimer timer;
    timer.start();
    std::vector<std::thread> writers;
    for (int t = 0; t < kThreads; ++t) {
        writers.emplace_back([] {
            for (int i = 0; i < kPerThread; ++i) {
                qInfo() << "throughput benchmark line";
            }
        });
    }
    for (auto &writer : writers) {
        writer.join();
    }
    const auto enqueueMs = timer.elapsed();
    logger->shutdown();
    const auto totalMs = timer.elapsed();

    // shutdown 后走 Qt 默认处理器，直接打到 stderr
    qWarning() << "throughput:" << kThreads * kPerThread << "messages, enqueue" << enqueueMs
               << "ms, drain total" << totalMs << "ms";

    QCOMPARE(readAllLines(m_dir->path()).size(), qsizetype(kThreads * kPerThread));
}

void AsyncLogUnitTest::testLogBufferBenchmark()
{
    LogBuffer buffer;
    QBENCHMARK
    {
        buffer.push(LogRecord{.message = QStringLiteral("benchmark line")});
        std::vector<LogRecord> consumed;
        (void) buffer.popBatch(consumed, std::chrono::milliseconds{10});
    }
}

QTEST_MAIN(AsyncLogUnitTest)
#include "asynclog_unittest.moc"
