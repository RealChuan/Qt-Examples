#ifndef LOGASYNC_H
#define LOGASYNC_H

#include <QMutex>
#include <QThread>

class LogAsync : public QThread
{
    Q_OBJECT
public:
    enum Orientation { Std = 1, File, StdAndFile };

    static auto instance() -> LogAsync *;

    void setLogPath(const QString &path);
    auto logPath() -> QString;

    void setAutoDelFile(bool on);
    auto autoDelFile() -> bool;

    void setAutoDelFileDays(qint64 days);
    auto autoDelFileDays() -> qint64;

    void setOrientation(Orientation /*orientation*/);
    auto orientation() -> Orientation;

    void setLogLevel(QtMsgType /*type*/);
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

    class LogAsyncPrivate;
    QScopedPointer<LogAsyncPrivate> d_ptr;
};

#endif // LOGASYNC_H
