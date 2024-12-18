#include "logfile.hpp"
#include "logasync.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QTimer>

#define ROLLSIZE (1000 * 1000 * 1000)

const static int g_kRollPerSeconds = 60 * 60 * 24;

static auto getFileName(qint64 seconds) -> QString
{
    auto data = QDateTime::fromSecsSinceEpoch(seconds).toString("yyyy-MM-dd-hh-mm-ss");
    auto filename = QString("%1/%2.%3.%4.%5.log")
                        .arg(LogAsync::instance()->logPath(),
                             qAppName(),
                             data,
                             QSysInfo::machineHostName(),
                             QString::number(qApp->applicationPid()));
    return filename;
}

static void autoDelFile()
{
    auto *instance = LogAsync::instance();
    const auto path(instance->logPath());
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    const auto list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    const auto cur = QDateTime::currentDateTime();
    const auto pre = cur.addDays(-instance->autoDelFileDays());

    for (const auto &info : std::as_const(list)) {
        if (info.lastModified() <= pre) {
            dir.remove(info.fileName());
        }
    }
}

class LogFile::LogFilePrivate
{
public:
    explicit LogFilePrivate(LogFile *q)
        : q_ptr(q)
    {}

    LogFile *q_ptr;

    QFile file;
    //QTextStream 读写分离的，内部有缓冲区static const int QTEXTSTREAM_BUFFERSIZE = 16384;
    QTextStream stream;
    qint64 startTime = 0;
    qint64 lastRoll = 0;
    int count = 0;
};

LogFile::LogFile(QObject *parent)
    : QObject(parent)
    , d_ptr(new LogFilePrivate(this))
{
    rollFile(0);
    setTimer();
}

LogFile::~LogFile()
{
    onFlush();
}

void LogFile::onWrite(const QString &msg)
{
    if (d_ptr->file.size() > ROLLSIZE) {
        rollFile(++d_ptr->count);
    } else {
        qint64 now = QDateTime::currentSecsSinceEpoch();
        qint64 thisPeriod = now / g_kRollPerSeconds * g_kRollPerSeconds;
        if (thisPeriod != d_ptr->startTime) {
            d_ptr->count = 0;
            rollFile(0);
        }
    }

    d_ptr->stream << msg;
}

void LogFile::onFlush()
{
    d_ptr->stream.flush();
}

auto LogFile::rollFile(int count) -> bool
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    QString filename = getFileName(now);
    if (count != 0) {
        filename += QString(".%1").arg(count);
    } else if (LogAsync::instance()->autoDelFile()) {
        autoDelFile();
    }
    qint64 start = now / g_kRollPerSeconds * g_kRollPerSeconds;
    if (now > d_ptr->lastRoll) {
        d_ptr->startTime = start;
        d_ptr->lastRoll = now;
        if (d_ptr->file.isOpen()) {
            d_ptr->file.flush();
            d_ptr->file.close();
        }
        d_ptr->file.setFileName(filename);
        d_ptr->file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
        d_ptr->stream.setDevice(&d_ptr->file);
        fprintf(stderr, "%s\n", filename.toUtf8().constData());
        return true;
    }
    return false;
}

void LogFile::setTimer()
{
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &LogFile::onFlush);
    timer->start(5000); // 5秒刷新一次
}
