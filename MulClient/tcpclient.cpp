#include "tcpclient.h"

#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    :QTcpSocket(parent)
{

}

TcpClient::~TcpClient()
{
    qDebug() << "~TcpClient";
}

auto TcpClient::getInfo() const -> QString
{
    QString info = peerName() + " " + peerAddress().toString()
            + " " + QString::number(peerPort()) + " ";
    return  info;
}

void TcpClient::onWrite(const QByteArray &buf)
{
    write(buf);
    waitForBytesWritten(3000);
}

void TcpClient::onReadyRead()
{
    QByteArray buf = readAll();
    QString str =  getInfo() + buf;
    //qDebug() << "onReadyRead: " << str;
}
