#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QThread>

class LogAsyncPrivate;
class LogAsync : public QThread
{
    Q_OBJECT
public:
    static LogAsync* instance();

    void setLogLevel(QtMsgType);    //日志级别
    void startWork();
    void finish();

signals:
    void appendBuf(const QString&);

protected:
    void run() override;

private:
    LogAsync(QObject *parent = nullptr);
    ~LogAsync() override;

    QScopedPointer<LogAsyncPrivate> d;
};

#endif // LOGASYNC_H
