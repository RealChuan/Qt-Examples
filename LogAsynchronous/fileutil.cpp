#include "fileutil.h"

#include <QDateTime>
#include <QFile>
#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QTextStream>

#define ROLLSIZE 1000*1000*1000

const static int kRollPerSeconds_ = 60*60*24;

class FileUtilPrivate{
public:
    FileUtilPrivate(QObject *parent) : owner(parent){}
    QObject *owner;
    QFile file;
    QTextStream stream;
    qint64 startTime = 0;
    qint64 lastRoll = 0;
    int count = 0;
    qint64 autoDelFileDays = 30;
};

FileUtil::FileUtil(qint64 days, QObject *parent) : QObject(parent)
  , d(new FileUtilPrivate(this))
{
    d->autoDelFileDays = days;
    newDir("log");
    rollFile(0);
}

FileUtil::~FileUtil()
{
    d->stream.flush();
    if(d->file.isOpen()){
        d->file.flush();
        d->file.close();
    }
    delete d;
}

void FileUtil::write(const QString &msg)
{
    if(d->file.size() > ROLLSIZE){
        rollFile(++d->count);
    }else{
        qint64 now = QDateTime::currentSecsSinceEpoch();
        qint64 thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
        if(thisPeriod != d->startTime){
            d->count = 0;
            rollFile(0);
            autoDelFile();
        }
    }

    d->stream << msg;
    //d->file.write(msg.toLocal8Bit().constData());
}

void FileUtil::newDir(const QString &path)
{
    QDir dir;
    if(!dir.exists(path))
        dir.mkdir(path);
}

QString FileUtil::getFileName(qint64* now) const
{
    *now = QDateTime::currentSecsSinceEpoch();
    QString data = QDateTime::fromSecsSinceEpoch(*now).toString("yyyy-MM-dd-hh-mm-ss");
    QString filename = QString("./log/%1.%2.%3.%4.log").arg(qAppName()).
            arg(data).arg(QSysInfo::machineHostName()).arg(qApp->applicationPid());
    return filename;
}

bool FileUtil::rollFile(int count)
{
    qint64 now = 0;
    QString filename = getFileName(&now);
    if(count){
        filename += QString(".%1").arg(count);
    }
    qint64 start = now / kRollPerSeconds_ * kRollPerSeconds_;
    if (now > d->lastRoll)
    {
        d->startTime = start;
        d->lastRoll = now;
        if(d->file.isOpen()){
            d->file.flush();
            d->file.close();
        }
        d->file.setFileName(filename);
        d->file.open(QIODevice::WriteOnly | QIODevice::Append |
                     QIODevice::Unbuffered);
        d->stream.setDevice(&d->file);
        fprintf(stderr, "%s\n", filename.toLocal8Bit().constData());
        return true;
    }
    return false;
}

void FileUtil::autoDelFile()
{
    newDir("log");
    QDir dir("./log/");

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);
    QDateTime cur = QDateTime::currentDateTime();
    QDateTime pre = cur.addDays(-d->autoDelFileDays);

    for(QFileInfo info : list){
        QDateTime birthTime = info.lastModified();
        if(birthTime <= pre)
            dir.remove(info.fileName());
    }
}
