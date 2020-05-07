#include "logasync.h"
#include "fileutil.h"

#include <QDateTime>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>

#define MAXLEN 1000*1000*10

static LogAsync *g_LogAsync = nullptr;
static QtMsgType MSGTYPE = QtWarningMsg;

// 消息处理函数
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if(type < MSGTYPE) return;

    QString level;
    switch(type)
    {
    case QtDebugMsg:    level = QLatin1String("Debug"); break;
    case QtWarningMsg:  level = QLatin1String("Warning"); break;
    case QtCriticalMsg: level = QLatin1String("Critica"); break;
    case QtFatalMsg:    level = QLatin1String("Fatal"); break;
    case QtInfoMsg:     level = QLatin1String("Info"); break;
    }

    const QDateTime dataTime = QDateTime::currentDateTime();
    const QString dataTimeString = dataTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString threadId = QString::number(qulonglong(QThread::currentThreadId()));

    QString contexInfo;
    contexInfo = QString("File:(%1) Line:(%2)").arg(
                QString(context.file),
                QString::number(context.line));

    const QString message = QString("%1 %2 [%3] %4 - %5\n")
            .arg(dataTimeString, threadId, level, msg, contexInfo);

    if(g_LogAsync)
        g_LogAsync->appendBuf(message);
    else{
        fprintf(stderr, "%s\n", "g_LogAsync = nullptr");
        fprintf(stderr, "%s", message.toLocal8Bit().constData());
    }
}

class LogAsyncPrivate{
public:
    LogAsyncPrivate(QObject *parent) : owner(parent){}
    QObject *owner;
    volatile bool running = false;
    QString currentBuf;
    QVector<QString> buffers;
    QVector<QString> bufferToWrite;
    QMutex mutex;
    QWaitCondition cond;
    unsigned long flushInterval;
};

LogAsync::LogAsync(unsigned long flushInterval, QObject *parent) : QThread(parent)
  , d(new LogAsyncPrivate(this))
{
    qInstallMessageHandler(messageHandler);
    d->flushInterval = flushInterval;
    d->running = true;
    if(nullptr != g_LogAsync){
        delete  g_LogAsync;
        g_LogAsync = nullptr;
    }
    g_LogAsync = this;
}

LogAsync::~LogAsync()
{
    d->running = false;
    if(isRunning()){
        quit();
        wait();
    }
    g_LogAsync = nullptr;
    qInstallMessageHandler(nullptr);
    delete d;
    fprintf(stderr, "%s\n", "~LogAsync");
}

void LogAsync::appendBuf(const QString &buf)
{
    QMutexLocker lock(&d->mutex);
    if(d->currentBuf.size() + buf.size() <= MAXLEN){
        d->currentBuf.append(buf);
    }else{
        d->buffers.push_back(d->currentBuf);
        d->currentBuf.clear();
        d->currentBuf.append(buf);
        d->cond.wakeOne();
    }
}

void LogAsync::setLogLevel(QtMsgType type)
{
    MSGTYPE = type;
}

void LogAsync::run()
{
    FileUtil fileUtil;
    while(d->running){
        {
            QMutexLocker lock(&d->mutex);
            if(d->buffers.isEmpty())
                d->cond.wait(&d->mutex, d->flushInterval);
            d->buffers.push_back(d->currentBuf);
            d->currentBuf.clear();
            d->bufferToWrite.swap(d->buffers);
            d->buffers.clear();
        }
        if(d->bufferToWrite.isEmpty()) continue;
        if(d->bufferToWrite.size() > 25){
            d->bufferToWrite.erase(d->bufferToWrite.begin() + 2, d->bufferToWrite.end());
        }
        for(const QString& buf: d->bufferToWrite){
            fileUtil.write(buf);        //输出到文件
        }
        for(int i=0; i < d->bufferToWrite.size(); i++)
            d->bufferToWrite.pop_back();
        d->bufferToWrite.clear();
    }
}
