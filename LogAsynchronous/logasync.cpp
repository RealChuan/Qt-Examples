#include "logasync.h"
#include "logfile.hpp"

#include <QDateTime>

#include <semaphore>

// 消息处理函数
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    auto *instance = LogAsync::instance();
    if (type < instance->logLevel()) {
        return;
    }

    FILE *stdPrint = stdout;
    QString level;
    switch (type) {
    case QtDebugMsg:
        level = QString("%1").arg("Debug", -7);
        stdPrint = stdout;
        break;
    case QtWarningMsg:
        level = QString("%1").arg("Warning", -7);
        stdPrint = stderr;
        break;
    case QtCriticalMsg:
        level = QString("%1").arg("Critica", -7);
        stdPrint = stderr;
        break;
    case QtFatalMsg:
        level = QString("%1").arg("Fatal", -7);
        stdPrint = stderr;
        break;
    case QtInfoMsg:
        level = QString("%1").arg("Info", -7);
        stdPrint = stdout;
        break;
    default: level = QString("%1").arg("Unknown", -7); break;
    }

    const auto dataTimeString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    const auto threadId = QString("%1").arg(reinterpret_cast<quint64>(QThread::currentThreadId()),
                                            5,
                                            10,
                                            QLatin1Char('0'));
    // By default, this information is recorded only in debug builds.
    // You can overwrite this explicitly by defining QT_MESSAGELOGCONTEXT or QT_NO_MESSAGELOGCONTEXT.
    QString contexInfo;
#ifndef QT_NO_DEBUG
    contexInfo = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
#endif
    const auto printToFile = QString("%1 %2 [%3] %4 - %5\n")
                                 .arg(dataTimeString, threadId, level, msg, contexInfo);
    QString printToConsole;
    if (msg.size() > instance->maxConsoleLineSize()) {
        printToConsole = QString("%1 %2 [%3] %4 - %5\n")
                             .arg(dataTimeString,
                                  threadId,
                                  level,
                                  msg.left(instance->maxConsoleLineSize()),
                                  contexInfo);
    } else {
        printToConsole = printToFile;
    }

    switch (instance->orientation()) {
    case LogAsync::Orientation::File: emit instance->appendBuf(printToFile); break;
    case LogAsync::Orientation::StandardAndFile:
        emit instance->appendBuf(printToFile);
        fprintf(stdPrint, "%s", printToConsole.toLocal8Bit().constData());
        ::fflush(stdPrint);
        break;
    default:
        fprintf(stdPrint, "%s", printToConsole.toLocal8Bit().constData());
        ::fflush(stdPrint);
        break;
    }
}

class LogAsync::LogAsyncPrivate
{
public:
    explicit LogAsyncPrivate(LogAsync *q)
        : q_ptr(q)
    {}

    LogAsync *q_ptr;

    QString logPath;
    bool autoDelFile = false;
    qint64 autoDelFileDays = 7;
    QtMsgType msgType = QtWarningMsg;
    LogAsync::Orientation orientation = LogAsync::Orientation::Standard;
    int maxConsoleLineSize = 1024 * 10;
    std::binary_semaphore semaphore{0};
};

auto LogAsync::instance() -> LogAsync *
{
    static LogAsync instance;
    return &instance;
}

void LogAsync::setLogPath(const QString &path)
{
    d_ptr->logPath = path;
}

auto LogAsync::logPath() -> QString
{
    return d_ptr->logPath;
}

void LogAsync::setAutoDelFile(bool on)
{
    d_ptr->autoDelFile = on;
}

auto LogAsync::autoDelFile() -> bool
{
    return d_ptr->autoDelFile;
}

void LogAsync::setAutoDelFileDays(qint64 days)
{
    d_ptr->autoDelFileDays = days;
}

auto LogAsync::autoDelFileDays() -> qint64
{
    return d_ptr->autoDelFileDays;
}

void LogAsync::setOrientation(LogAsync::Orientation orientation)
{
    d_ptr->orientation = orientation;
}

auto LogAsync::orientation() -> LogAsync::Orientation
{
    return d_ptr->orientation;
}

void LogAsync::setLogLevel(QtMsgType type)
{
    d_ptr->msgType = type;
}

auto LogAsync::logLevel() -> QtMsgType
{
    return d_ptr->msgType;
}

void LogAsync::setMaxConsoleLineSize(int size)
{
    if (size < 1) {
        return;
    }
    d_ptr->maxConsoleLineSize = size;
}

auto LogAsync::maxConsoleLineSize() -> int
{
    return d_ptr->maxConsoleLineSize;
}

void LogAsync::startWork()
{
    start();
    d_ptr->semaphore.acquire();
}

void LogAsync::stop()
{
    // 可能有部分日志未写入文件，异步信号槽机制问题
    if (isRunning()) {
        quit();
        wait();
    }
}

void LogAsync::run()
{
    LogFile logFile;
    connect(this, &LogAsync::appendBuf, &logFile, &LogFile::onWrite);
    d_ptr->semaphore.release();
    exec();
}

LogAsync::LogAsync(QObject *parent)
    : QThread(parent)
    , d_ptr(new LogAsyncPrivate(this))
{
    qInstallMessageHandler(messageHandler);
}

LogAsync::~LogAsync()
{
    stop();
    qInstallMessageHandler(nullptr);
    fprintf(stderr, "%s\n", "~LogAsync");
}
