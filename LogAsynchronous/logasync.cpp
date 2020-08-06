#include "logasync.h"
#include "fileutil.h"

#include <QDateTime>
#include <QWaitCondition>
#include <QMutex>

static QtMsgType g_MsgType = QtWarningMsg;

// 消息处理函数
void messageHandler(QtMsgType type, const QMessageLogContext &context
                    , const QString &msg)
{
    if(type < g_MsgType) return;

    QString level;
    switch(type){
    case QtDebugMsg:    level = QLatin1String("Debug"); break;
    case QtWarningMsg:  level = QLatin1String("Warning"); break;
    case QtCriticalMsg: level = QLatin1String("Critica"); break;
    case QtFatalMsg:    level = QLatin1String("Fatal"); break;
    case QtInfoMsg:     level = QLatin1String("Info"); break;
    }

    const QString dataTimeString = QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString threadId = QString::number(qulonglong(QThread::currentThreadId()));
    const QString contexInfo = QString("File(%1) Line(%2)")
            .arg(QString(context.file),QString::number(context.line));

    const QString message = QString("%1 %2 [%3] %4 - %5\n")
            .arg(dataTimeString, threadId, level, msg, contexInfo);

    LogAsync::instance()->appendBuf(message);
}

class LogAsyncPrivate{
public:
    LogAsyncPrivate(QThread *owner) : owner(owner){}
    QThread *owner;
    QWaitCondition waitCondition;
    QMutex mutex;
};

static QMutex mutex;

LogAsync *LogAsync::instance()
{
    QMutexLocker locker(&mutex);
    static LogAsync log;
    return &log;
}

void LogAsync::setLogLevel(QtMsgType type)
{
    g_MsgType = type;
}

void LogAsync::startWork()
{
    start();
    QMutexLocker lock(&d->mutex);
    d->waitCondition.wait(&d->mutex);
}

void LogAsync::stop()
{
    if(isRunning()){
        QThread::msleep(500);   // 最后一条日志格式化可能来不及进入信号槽
        quit();
        wait();
    }
}

void LogAsync::run()
{
    FileUtil fileUtil;
    connect(this, &LogAsync::appendBuf, &fileUtil, &FileUtil::onWrite);
    d->waitCondition.wakeOne();
    exec();
}

LogAsync::LogAsync(QObject *parent) : QThread(parent)
  , d(new LogAsyncPrivate(this))
{
    qInstallMessageHandler(messageHandler);
}

LogAsync::~LogAsync()
{
    stop();
    qInstallMessageHandler(nullptr);
    fprintf(stderr, "%s\n", "~LogAsync");
}
