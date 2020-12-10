#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QThread>

class LogAsyncPrivate;
class LogAsync : public QThread
{
    Q_OBJECT
public:
    enum Orientation { Std = 1, File, StdAndFile};

    static LogAsync* instance();

    void setOrientation(Orientation);
    void setLogLevel(QtMsgType);    //日志级别
    void startWork();
    void stop();

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
