#include "logasync.h"
#include "fileutil.h"

#include <QDateTime>
#include <QWaitCondition>

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

    const QString dataTimeString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    const QString threadId = QString("%1").arg(reinterpret_cast<quint64>(QThread::currentThreadId()),
                                               5,
                                               10,
                                               QLatin1Char('0'));
    // By default, this information is recorded only in debug builds.
    // You can overwrite this explicitly by defining QT_MESSAGELOGCONTEXT or QT_NO_MESSAGELOGCONTEXT.
    QString contexInfo;
#ifndef QT_NO_DEBUG
    contexInfo = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
#endif
    const auto message = QString("%1 %2 [%3] %4 - %5\n")
                             .arg(dataTimeString, threadId, level, msg, contexInfo);

    switch (instance->orientation()) {
    case LogAsync::Orientation::Std:
        fprintf(stdPrint, "%s", message.toLocal8Bit().constData());
        ::fflush(stdPrint);
        break;
    case LogAsync::Orientation::File: emit instance->appendBuf(message); break;
    case LogAsync::Orientation::StdAndFile:
        fprintf(stdPrint, "%s", message.toLocal8Bit().constData());
        ::fflush(stdPrint);
        emit instance->appendBuf(message);
        break;
    default:
        fprintf(stdPrint, "%s", message.toLocal8Bit().constData());
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
    LogAsync::Orientation orientation = LogAsync::Orientation::Std;
    QWaitCondition waitCondition;
    QMutex mutex;
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

void LogAsync::startWork()
{
    start();
    QMutexLocker lock(&d_ptr->mutex);
    d_ptr->waitCondition.wait(&d_ptr->mutex);
}

void LogAsync::stop()
{
    if (isRunning()) {
        //QThread::sleep(1);   // 最后一条日志格式化可能来不及进入信号槽
        quit();
        wait();
    }
}

void LogAsync::run()
{
    FileUtil fileUtil;
    connect(this, &LogAsync::appendBuf, &fileUtil, &FileUtil::onWrite);
    d_ptr->waitCondition.wakeOne();
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
