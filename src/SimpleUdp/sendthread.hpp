#ifndef SENDTHREAD_HPP
#define SENDTHREAD_HPP

#include <QThread>
#include <QtCore/qglobal.h>

// 广播类
class SendThread : public QThread
{
    Q_OBJECT
public:
    explicit SendThread(QObject *parent = nullptr);
    ~SendThread() override;

public slots:
    void onStart();
    void onStop();

protected:
    void run() override;

private:
    struct SendThreadPrivate;
    QScopedPointer<SendThreadPrivate> d_ptr;
};

#endif // SENDTHREAD_HPP
