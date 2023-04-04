#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QMutex>
#include <QThread>

struct LogAsyncPrivate;
class LogAsync : public QThread
{
    Q_OBJECT
public:
    enum Orientation { Std = 1, File, StdAndFile };

    static auto instance() -> LogAsync *;

    void setOrientation(Orientation);
    auto orientation() -> Orientation;

    void setLogLevel(QtMsgType);
    auto logLevel() -> QtMsgType;

    void startWork();
    void stop();

signals:
    void appendBuf(const QString &);

protected:
    void run() override;

private:
    explicit LogAsync(QObject *parent = nullptr);
    ~LogAsync() override;

    static QMutex m_mutex;
    QScopedPointer<LogAsyncPrivate> d_ptr;
};

#endif // LOGASYNC_H
