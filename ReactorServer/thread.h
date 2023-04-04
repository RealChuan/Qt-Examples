#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class TcpClient;
class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(QObject *parent = nullptr);
    ~Thread() override;

signals:
    void newConnectHandle(qintptr);
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void run() override;
};

#endif // THREAD_H
