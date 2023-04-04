#include "tcpclient.h"

#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    :QTcpSocket(parent)
{
}

TcpClient::~TcpClient()
{
    qDebug() << "~TcpSocket";
}

auto TcpClient::getInfo() const -> QString
{
    QString info = peerName() + " " + peerAddress().toString()
            + " " + QString::number(peerPort()) + " ";
    return info;
}

void TcpClient::onReadyRead()
{
    //这里如果有复杂的计算可以放入QThreadPool中，以免下一个信号排队等待
    QByteArray buf = readAll();
    write(buf);
    waitForBytesWritten(3000);
    //QString str =  getInfo() + buf;
    //qDebug() << "onReadyRead: " << str;
}
