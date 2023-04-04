#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class TcpClient;
class Thread : public QThread
{
    Q_OBJECT
public:
    explicit Thread(qintptr socketDescriptor, QObject *parent = nullptr);
    ~Thread() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void run() override;

private:
    qintptr socketfd;
};

#endif // THREAD_H
