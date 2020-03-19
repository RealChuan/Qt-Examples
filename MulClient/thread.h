#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class Thread : public QThread
{
    Q_OBJECT
public:
    Thread(const QString&, int, QObject *parent = nullptr);
    ~Thread() override;

signals:
    void writeToServer(const QByteArray&);

protected:
    void run() override;

private:
    QString ip;
    int port;
};

#endif // THREAD_H
