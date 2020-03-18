#include "subreactor.h"
#include "thread.h"
#include "tcpsocket.h"

static QAtomicInt totalCount = 0;

SubReactor::SubReactor(QObject *parent)
    :QObject(parent)
{
}

SubReactor::~SubReactor()
{
    qDebug() << "~SubReactor";
}

void SubReactor::onNewConnect(qintptr socketfd)
{
    qDebug() << "Test onNewConnect: " << QThread::currentThreadId();
    TcpSocket *tcpSocket = new TcpSocket(this);
    if(!tcpSocket->setSocketDescriptor(socketfd)){
        qWarning() << tr("connection failed fd: ") << socketfd
                   << tcpSocket->errorString();
        return;
    }
    count++;
    totalCount.fetchAndAddOrdered(1);
    qDebug() << "onNewConnect: " << totalCount;
    connect(tcpSocket, &TcpSocket::readyRead,
            tcpSocket, &TcpSocket::onReadyRead, Qt::DirectConnection);
    connect(tcpSocket, &TcpSocket::message, this, &SubReactor::message);
    connect(tcpSocket, &TcpSocket::disconnected,
            tcpSocket, &TcpSocket::deleteLater, Qt::DirectConnection);
    connect(tcpSocket, &TcpSocket::disconnected, this, &SubReactor::onClose, Qt::DirectConnection);
    QString str = tr("Client online: ") + tcpSocket->getInfo();
    emit message(str);
    emit maxCount(count);
    emit clientCount(count);
}

void SubReactor::onClose()
{
    count--;
    totalCount.fetchAndSubOrdered(1);
    QString str = tr("Client Offline Current quantity: ") +
            QString::number(totalCount);
    emit message(str);
    emit clientCount(totalCount);
}
