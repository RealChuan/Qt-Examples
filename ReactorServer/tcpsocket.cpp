#include "tcpsocket.h"

#include <QHostAddress>

TcpSocket::TcpSocket(QObject *parent)
    :QTcpSocket(parent)
{
}

TcpSocket::~TcpSocket()
{
    qDebug() << tr("Client Offline!");
    qDebug() << "~TcpSocket";
}

QString TcpSocket::getInfo() const
{
    QString info = peerName() + " " + peerAddress().toString()
            + " " + QString::number(peerPort()) + " ";
    return info;
}

void TcpSocket::onReadyRead()
{
    //这里如果有复杂的计算可以放入QThreadPool中，以免下一个信号排队等待
    QByteArray buf = readAll();
    write(buf);
    waitForBytesWritten(3000);
    QString str =  getInfo() + buf;
    emit message(str);
    //qDebug() << "onReadyRead: " << str;
}
