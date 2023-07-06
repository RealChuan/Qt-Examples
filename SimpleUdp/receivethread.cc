#include "receivethread.hpp"

#include <QCoreApplication>
#include <QNetworkDatagram>
#include <QUdpSocket>

struct ReceiveThread::ReceiveThreadPrivate
{
    const quint16 recvPort = 6000;
    volatile bool runing = true;
};

ReceiveThread::ReceiveThread(QObject *parent)
    : QThread{parent}
    , d_ptr(new ReceiveThreadPrivate)
{}

ReceiveThread::~ReceiveThread()
{
    onStop();
}

void ReceiveThread::onStart()
{
    d_ptr->runing = true;
    if (isRunning()) {
        return;
    }
    start();
}

void ReceiveThread::onStop()
{
    d_ptr->runing = false;
    if (isRunning()) {
        quit();
        wait();
    }
}

void ReceiveThread::run()
{
    qInfo() << "Start Receive-----------------";
    QScopedPointer<QUdpSocket> recvUdpSocket(new QUdpSocket);
    // The recvUdpSocket.data() must no buffer data to read before bind, otherwise the readyRead signal will not be emitted.
    // connect(recvUdpSocket.data(),
    //         &QUdpSocket::readyRead,
    //         this,
    //         &UdpReceiveThread::onReadyRead,
    //         Qt::DirectConnection);
    auto bind = recvUdpSocket->bind(QHostAddress::Any, d_ptr->recvPort, QUdpSocket::ShareAddress);
    if (bind) {
        qInfo() << "bind success";
    } else {
        qInfo() << "bind failed";
        return;
    }
    while (d_ptr->runing) {
        recvUdpSocket->waitForReadyRead(100);
        while (d_ptr->runing && recvUdpSocket->hasPendingDatagrams()) {
            QNetworkDatagram datagram = recvUdpSocket->receiveDatagram();
            processTheDatagram(datagram);
        }
        qApp->processEvents();
    }
    qInfo() << "Stop Receive-----------------";
}

void ReceiveThread::processTheDatagram(const QNetworkDatagram &datagram)
{
    qInfo() << "Receive From:" << datagram.senderAddress().toString() << datagram.senderPort()
            << "To:" << datagram.destinationAddress() << datagram.destinationPort()
            << datagram.data();
}
