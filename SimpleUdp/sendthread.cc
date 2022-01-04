#include "sendthread.hpp"

#include <QNetworkInterface>
#include <QUdpSocket>

struct SendThread::SendThreadPrivate
{
    volatile bool runing = true;
    const quint16 sendPort = 6000;
};

SendThread::SendThread(QObject *parent)
    : QThread{parent}
    , d_ptr(new SendThreadPrivate)
{}

SendThread::~SendThread()
{
    onStop();
}

void SendThread::onStart()
{
    d_ptr->runing = true;
    if (isRunning()) {
        return;
    }
    start();
}

void SendThread::onStop()
{
    d_ptr->runing = false;
    if (isRunning()) {
        quit();
        wait();
    }
}

void SendThread::run()
{
    QScopedPointer<QUdpSocket> sendUdpSocket(new QUdpSocket);
    int loop = 3; // 发送多次
    qInfo() << "Start Send-----------------";
    while (d_ptr->runing && loop > 0) {
        const QByteArray buf = "Hello " + QByteArray::number(loop);
        QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : qAsConst(interfaceList)) {
            QList<QNetworkAddressEntry> entryList = interface.addressEntries();
            for (const QNetworkAddressEntry &entry : qAsConst(entryList)) {
                const QHostAddress broadcastAdress = entry.broadcast();
                if (broadcastAdress == QHostAddress::Null
                    /*|| broadcastAdress == QHostAddress::LocalHost*/) {
                    continue;
                }
                sendUdpSocket->writeDatagram(buf, broadcastAdress, d_ptr->sendPort);
                qInfo() << "Send To:" << broadcastAdress << buf;
                sendUdpSocket->flush();
            }
        }
        sendUdpSocket->writeDatagram(buf, QHostAddress::Broadcast, d_ptr->sendPort);
        sendUdpSocket->flush();
        QThread::msleep(100);
        loop--;
    }
    qInfo() << "Stop Send-----------------";
}
