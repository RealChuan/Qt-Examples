#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QThread>

class LogAsyncPrivate;
class LogAsync : public QThread
{
    Q_OBJECT
public:
    LogAsync(unsigned long flushInterval = 5000, QObject *parent = nullptr);
    ~LogAsync() override;

    void appendBuf(const QString&);
    void setLogLevel(QtMsgType);    //日志级别

protected:
    void run() override;

private:
    LogAsyncPrivate *d;
};

#endif // LOGASYNC_H
