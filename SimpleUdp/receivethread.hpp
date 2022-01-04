#ifndef RECEIVETHREAD_HPP
#define RECEIVETHREAD_HPP

#include <QThread>
#include <QtCore/qglobal.h>

class QNetworkDatagram;

// 接收类
class ReceiveThread : public QThread
{
    Q_OBJECT
public:
    explicit ReceiveThread(QObject *parent = nullptr);
    ~ReceiveThread() override;

public slots:
    void onStart();
    void onStop();

protected:
    void run() override;

private:
    void processTheDatagram(const QNetworkDatagram &datagram);

    struct ReceiveThreadPrivate;
    QScopedPointer<ReceiveThreadPrivate> d_ptr;
};

#endif // RECEIVETHREAD_HPP
