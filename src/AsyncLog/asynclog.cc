#include "asynclog.hpp"
#include "logbuffer.hpp"
#include "logsink.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaMethod>
#include <QRegularExpression>
#include <QSysInfo>
#include <QtGlobal>
#include <QThread>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <expected>
#include <format>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace AsyncLog {

namespace {

// 行格式：2026-09-27 14:30:05.123 [I] [t 1a2b] [category] message (file.cc:42)
// 级别单字母，索引为 severity()；第 6 位 '?' 兜底未知类型
constexpr char kLevelTag[] = "DIWCF?";

// 秒级时间戳缓存：同一秒内的所有行复用已格式化文本，毫秒独立拼接
class TimestampFormatter
{
public:
    [[nodiscard]] std::string_view seconds(qint64 ns)
    {
        const qint64 sec = ns / qint64{1'000'000'000};
        if (sec != m_lastSec) {
            m_lastSec = sec;
            m_text
                = QDateTime::fromSecsSinceEpoch(sec).toString("yyyy-MM-dd hh:mm:ss").toStdString();
        }
        return m_text;
    }

private:
    qint64 m_lastSec = -1;
    std::string m_text;
};

// 组装一行 UTF-8（含换行符）到 out。仅后端线程与 fatal 同步路径调用，
// TimestampFormatter 按线程缓存，天然无共享
void appendLine(std::string &out, const LogRecord &record)
{
    thread_local TimestampFormatter formatter;
    const qint64 ms = (record.timestampNs % qint64{1'000'000'000}) / qint64{1'000'000};
    std::format_to(std::back_inserter(out),
                   "{}.{:03} [{}] [t {:x}] [{}] ",
                   formatter.seconds(record.timestampNs),
                   static_cast<quint64>(ms),
                   kLevelTag[severity(record.type)],
                   record.threadId,
                   record.category);
    const auto message = record.message.toUtf8();
    out.append(message.constData(), static_cast<std::size_t>(message.size()));
    if (record.file != nullptr) { // release 下 QT_MESSAGELOGCONTEXT 未定义时为 nullptr
        std::format_to(std::back_inserter(out), " ({}:{})", record.file, record.line);
    }
    out.push_back('\n');
}

// 文件名前缀：base_<version>。版本是应用设置的外部输入，落文件名前清洗
// （Windows 非法字符与空白统一替换为 '-'，首尾空白去除）；
// 清洗后为空（未设置）则填 unknown 占位，版本槽位恒存在
[[nodiscard]] QString makeNamePrefix(const QString &baseName, const QString &version)
{
    static const QRegularExpression invalid(R"([<>:"/\\|?*\s])");
    auto cleaned = version.trimmed();
    cleaned.replace(invalid, QStringLiteral("-"));
    return QString("%1_%2").arg(baseName, cleaned.isEmpty() ? QStringLiteral("unknown") : cleaned);
}

// —— FileSink：滚动文件 sink，纯内部实现，整个类只在 .cc 可见 ——
//
// 命名：<base>_<version>_<host>_<pid>_<yyyyMMdd_hhmmss>[_<seq>].log。版本号与
// 主机名、进程 ID 一起写进文件名：多机汇总日志时可定位来源主机与程序版本，
// 同主机并发实例互不混淆；版本取 applicationVersion，未设置时槽位填 unknown。
// 同秒重复滚动加序号，滚动不依赖时间比较，不存在同秒不滚动的缺陷。
// 按天 + 按大小滚动；过期清理只针对匹配自身命名模式的文件。
// 单文件可超出 maxFileSize 至多一批（写后检查），批级别近似，可接受
class FileSink final : public ISink
{
public:
    struct Params
    {
        QString dir;
        QString baseName;
        QString version; // 未设置时文件名版本槽位填 unknown
        qint64 maxFileSize = 512LL * 1024 * 1024;
        qint64 keepDays = 7;
    };

    // 目录不可用等可恢复错误通过 std::expected 返回，不抛异常
    [[nodiscard]] static auto create(const Params &params)
        -> std::expected<std::unique_ptr<FileSink>, QString>
    {
        QDir dir(params.dir);
        if (!dir.exists() && !dir.mkpath(".")) {
            return std::unexpected(QString("cannot create log directory: %1").arg(params.dir));
        }
        auto sink = std::make_unique<FileSink>(params);
        {
            std::lock_guard lock(sink->mutex);
            sink->rollFile(); // 打开首个日志文件，失败在下面报告
        }
        if (!sink->file.isOpen()) {
            return std::unexpected(QString("cannot open log file in: %1").arg(params.dir));
        }
        return sink;
    }

    ~FileSink() override { flush(); }

    explicit FileSink(const Params &p)
        : params(p), namePrefix(makeNamePrefix(p.baseName, p.version)), dir(p.dir)
    {}

    void write(std::span<const char> bytes, bool /*isError*/) override
    {
        std::lock_guard lock(mutex);
        pending.append(bytes.data(), static_cast<qsizetype>(bytes.size()));
    }

    void flush() override
    {
        std::lock_guard lock(mutex);
        if (pending.isEmpty()) {
            return;
        }

        // 跨天检查（批级别近似：一个批次内跨天界线的行归入新文件）
        const auto nowSecs = QDateTime::currentSecsSinceEpoch();
        if (nowSecs / 86400 != currentDay) {
            rollFile();
        }

        if (!file.isOpen()) {
            rollFile(); // 上次打开失败则重试
        }
        if (file.isOpen()) {
            // 磁盘写满等场景下 write 可能失败或部分成功；残留数据留在缓冲等下批重试
            const auto written = file.write(pending);
            if (written == pending.size()) {
                writtenBytes += written;
                pending.clear();
            } else if (written > 0) {
                writtenBytes += written;
                pending.remove(0, written);
            }
            file.flush();
        }
        if (writtenBytes > params.maxFileSize) {
            // 写完这批再滚动：不依赖时间比较，同一秒内重复滚动也正确
            rollFile();
        }
    }

    // 删除目录下所有匹配自身命名模式的历史日志（含其他进程实例留下的），
    // 正在写入的当前文件除外，返回删除的文件数。供 Logger::purgeLogFiles 调用。
    // 注：另一存活实例正在写的文件在 POSIX 上会被解除链接（该进程继续写入
    // 孤儿 inode），Windows 上删除失败不计数
    int purgeAll()
    {
        std::lock_guard lock(mutex);
        const auto namePattern = ownNamePattern();
        const auto currentName = QFileInfo(file.fileName()).fileName();
        const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        int removed = 0;
        for (const auto &info : files) {
            if (info.fileName() == currentName) {
                continue; // 正在写入的文件除外
            }
            if (namePattern.match(info.fileName()).hasMatch()
                && QFile::remove(info.absoluteFilePath())) {
                ++removed;
            }
        }
        return removed;
    }

private:
    // 调用方持有 mutex
    void rollFile()
    {
        if (file.isOpen()) {
            file.flush();
            file.close();
        }

        const auto nowSecs = QDateTime::currentSecsSinceEpoch();
        QString name = QString("%1_%2_%3_%4")
                           .arg(namePrefix,
                                hostName,
                                pid,
                                QDateTime::fromSecsSinceEpoch(nowSecs).toString("yyyyMMdd_hhmmss"));
        if (nowSecs == lastNameSec) {
            name += QString("_%1").arg(++rollSeq); // 同秒滚动加序号
        } else {
            rollSeq = 0;
            lastNameSec = nowSecs;
        }
        name += ".log";

        file.setFileName(dir.filePath(name));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            std::fprintf(
                stderr, "AsyncLog: cannot open log file: %s\n", qPrintable(file.errorString()));
            return;
        }
        writtenBytes = file.size();
        currentDay = nowSecs / 86400;

        purgeExpired();
    }

    // 自身命名模式：版本/主机/进程槽位用通配，
    // 本应用任意历史进程、任意历史版本留下的日志都算“自己的”
    [[nodiscard]] QRegularExpression ownNamePattern() const
    {
        return QRegularExpression(QRegularExpression::anchoredPattern(
            QString("%1_.+_\\d{8}_\\d{6}(_\\d+)?\\.log")
                .arg(QRegularExpression::escape(params.baseName))));
    }

    // 清理只针对匹配自身命名模式的过期日志，绝不波及目录里其他文件
    void purgeExpired()
    {
        if (params.keepDays <= 0) {
            return;
        }
        const auto namePattern = ownNamePattern();
        const auto cutoff = QDateTime::currentDateTime().addDays(-params.keepDays);
        const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const auto &info : files) {
            if (namePattern.match(info.fileName()).hasMatch() && info.lastModified() < cutoff) {
                QFile::remove(info.absoluteFilePath());
            }
        }
    }

    const Params params;
    const QString hostName = QSysInfo::machineHostName();                    // 汇总时定位主机
    const QString pid = QString::number(QCoreApplication::applicationPid()); // 区分同机多实例
    const QString namePrefix; // base_version（版本未设置时为 base_unknown）
    QDir dir;
    QFile file;
    QByteArray pending;      // 攒批缓冲，flush 时一次写入
    qint64 writtenBytes = 0; // 当前文件已写字节
    qint64 currentDay = 0;   // 当天时段（epoch 秒 / 86400）
    qint64 lastNameSec = 0;  // 上次命名的秒值，同秒加序号
    int rollSeq = 0;
    std::mutex mutex;        // fatal 同步路径可能从其他线程调用
};

} // namespace

class Logger::LoggerPrivate
{
public:
    explicit LoggerPrivate(Logger *q) : q_ptr(q) {}

    Logger *q_ptr;
    std::atomic<QtMsgType> level{QtInfoMsg};
    std::atomic<bool> running{false};

    LogBuffer buffer;
    std::vector<std::unique_ptr<ISink>> sinks;
    std::unique_ptr<std::thread> backend;

    FileSink *fileSink = nullptr; // 非 owning，sinks 持有所有权；仅运行期间非空
    std::mutex lifecycleMutex;    // 串行化 shutdown 与 purgeLogFiles 的状态调整
};

Logger *Logger::instance()
{
    static Logger inst;
    return &inst;
}

Logger::Logger(QObject *parent) : QObject(parent), d_ptr(std::make_unique<LoggerPrivate>(this)) {}

Logger::~Logger()
{ shutdown(); }

void Logger::start(const Config &config)
{
    auto *d = d_ptr.get();
    bool expected = false;
    if (!d->running.compare_exchange_strong(expected, true)) {
        return; // 已在运行，幂等
    }

    d->level.store(config.level, std::memory_order_relaxed);

    d->sinks.clear();
    if (config.console) {
        d->sinks.push_back(std::make_unique<ConsoleSink>(
            config.consoleTruncate > 0 ? static_cast<std::size_t>(config.consoleTruncate)
                                       : std::size_t{0}));
    }
    if (config.file) {
        auto sink = FileSink::create({.dir = config.logPath,
                                      .baseName = QCoreApplication::applicationName(),
                                      .version = QCoreApplication::applicationVersion(),
                                      .maxFileSize = config.maxFileSize,
                                      .keepDays = config.keepDays});
        if (sink) {
            d->fileSink = sink->get();
            d->sinks.push_back(std::move(*sink));
        } else {
            // 日志模块自身的问题直接走 stderr，绝不递归 qWarning
            std::fprintf(
                stderr, "AsyncLog: file sink disabled: %s\n", sink.error().toUtf8().constData());
        }
    }

    d->buffer.reopen();
    d->backend = std::make_unique<std::thread>([this, d] {
        std::vector<LogRecord> batch;
        std::string line;
        QStringList lines;
        while (d->buffer.popBatch(batch, std::chrono::seconds{3})) {
            // 无人订阅时跳过批次的字符串构建
            const bool emitBatch
                = this->isSignalConnected(QMetaMethod::fromSignal(&Logger::batchReady));
            lines.clear();
            for (auto &record : batch) {
                line.clear();
                appendLine(line, record);
                if (emitBatch) {
                    lines << QString::fromStdString(line);
                }
                const bool isError = severity(record.type) >= severity(QtWarningMsg);
                for (auto &sink : d->sinks) {
                    sink->write(std::span{line.data(), line.size()}, isError);
                }
            }
            if (const auto dropped = d->buffer.takeDropped(); dropped > 0) {
                line.clear();
                std::format_to(std::back_inserter(line), "... dropped {} messages ...\n", dropped);
                if (emitBatch) {
                    lines << QString::fromStdString(line);
                }
                for (auto &sink : d->sinks) {
                    sink->write(std::span{line.data(), line.size()}, false);
                }
            }
            for (auto &sink : d->sinks) {
                sink->flush();
            }
            if (!lines.isEmpty()) {
                emit this->batchReady(lines);
            }
            batch.clear();
        }
    });
    qInstallMessageHandler(&messageHandler); // 最后安装：就绪前的新消息走 Qt 默认路径
}

void Logger::shutdown()
{
    auto *d = d_ptr.get();
    bool expected = true;
    if (!d->running.compare_exchange_strong(expected, false)) {
        return; // 未运行，幂等
    }

    qInstallMessageHandler(nullptr); // 新消息回归 Qt 默认路径，不再入队
    d->buffer.close();               // 拒收并唤醒后端做最终排空
    if (d->backend && d->backend->joinable()) {
        d->backend->join();          // 后端排空后自行退出；std::thread 不 join 会 terminate
    }
    d->backend.reset();
    {
        // 持锁析构 sinks：与 purgeLogFiles 互斥，FileSink 不会被并发使用/析构
        std::lock_guard lock(d->lifecycleMutex);
        d->fileSink = nullptr;
        d->sinks.clear(); // FileSink 析构时 flush 残余并关文件
    }
}

void Logger::setLevel(QtMsgType level)
{ d_ptr->level.store(level, std::memory_order_relaxed); }

auto Logger::level() const -> QtMsgType
{ return d_ptr->level.load(std::memory_order_relaxed); }

auto Logger::isRunning() const -> bool
{ return d_ptr->running.load(std::memory_order_relaxed); }

auto Logger::purgeLogFiles() -> int
{
    auto *d = d_ptr.get();
    std::lock_guard lock(d->lifecycleMutex);
    if (d->fileSink == nullptr) {
        return 0; // 未运行或未启用文件输出
    }
    // 持锁调用：与 shutdown 的 sinks.clear() 互斥，FileSink 不会被并发析构
    return d->fileSink->purgeAll();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    auto *d = Logger::instance()->d_ptr.get();
    if (severity(type) < severity(d->level.load(std::memory_order_relaxed))) {
        return; // 未达输出级别，捕获端最低成本路径
    }

    LogRecord record{
        .timestampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count(),
        .type = type,
        .message = message,
        .threadId = reinterpret_cast<quintptr>(QThread::currentThreadId()),
        .category = context.category != nullptr ? context.category : "default",
        .file = context.file,
        .line = context.line,
    };

    if (type == QtFatalMsg) {
        // fatal 必须同步落盘：Qt 在处理器返回后立即 abort，异步队列来不及消费
        std::string line;
        appendLine(line, record);
        for (auto &sink : d->sinks) {
            sink->write(std::span{line.data(), line.size()}, true);
        }
        for (auto &sink : d->sinks) {
            sink->flush();
        }
        return;
    }

    d->buffer.push(std::move(record));
}

} // namespace AsyncLog
