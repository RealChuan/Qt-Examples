#include "fileutil.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QTimer>

#define ROLLSIZE 1000 * 1000 * 1000

const static int kRollPerSeconds_ = 60 * 60 * 24;

auto generateDirectorys(const QString &directory) -> bool
{
    QDir sourceDir(directory);
    if (sourceDir.exists()) {
        return true;
}

    QString tempDir;
    QStringList directorys = directory.split("/");
    for (int i = 0; i < directorys.count(); i++) {
        QString path = directorys[i];
        tempDir += path + "/";

        QDir dir(tempDir);
        if (!dir.exists() && !dir.mkdir(tempDir)) {
            return false;
}
    }

    return true;
}

struct FileUtilPrivate
{
    QFile file;
    //QTextStream 读写分离的，内部有缓冲区static const int QTEXTSTREAM_BUFFERSIZE = 16384;
    QTextStream stream;
    qint64 startTime = 0;
    qint64 lastRoll = 0;
    int count = 0;
    qint64 autoDelFileDays = 30;
};

FileUtil::FileUtil(qint64 days, QObject *parent)
    : QObject(parent)
    , d_ptr(new FileUtilPrivate)
{
    d_ptr->autoDelFileDays = days;
    generateDirectorys(qApp->applicationDirPath() + "/log");
    rollFile(0);
    setTimer();
}

FileUtil::~FileUtil()
{
    onFlush();
}

void FileUtil::onWrite(const QString &msg)
{
    if (d_ptr->file.size() > ROLLSIZE) {
        rollFile(++d_ptr->count);
    } else {
        qint64 now = QDateTime::currentSecsSinceEpoch();
        qint64 thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
        if (thisPeriod != d_ptr->startTime) {
            d_ptr->count = 0;
            rollFile(0);
        }
    }

    d_ptr->stream << msg;
}

void FileUtil::onFlush()
{
    d_ptr->stream.flush();
}

auto FileUtil::getFileName(qint64 *now) const -> QString
{
    *now = QDateTime::currentSecsSinceEpoch();
    QString data = QDateTime::fromSecsSinceEpoch(*now).toString("yyyy-MM-dd-hh-mm-ss");
    QString filename = QString("%1/log/%2.%3.%4.%5.log")
                           .arg(qApp->applicationDirPath(),
                                qAppName(),
                                data,
                                QSysInfo::machineHostName(),
                                QString::number(qApp->applicationPid()));
    return filename;
}

auto FileUtil::rollFile(int count) -> bool
{
    qint64 now = 0;
    QString filename = getFileName(&now);
    if (count) {
        filename += QString(".%1").arg(count);
    } else {
        autoDelFile();
    }
    qint64 start = now / kRollPerSeconds_ * kRollPerSeconds_;
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

void FileUtil::autoDelFile()
{
    const QString path(qApp->applicationDirPath() + "/log");
    generateDirectorys(path);
    QDir dir(path);

    const QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    const QDateTime cur = QDateTime::currentDateTime();
    const QDateTime pre = cur.addDays(-d_ptr->autoDelFileDays);

    for (const QFileInfo &info : qAsConst(list)) {
        if (info.lastModified() <= pre) {
            dir.remove(info.fileName());
        }
    }
}

void FileUtil::setTimer()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FileUtil::onFlush);
    timer->start(5000); // 5秒刷新一次
}
