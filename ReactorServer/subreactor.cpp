#include "subreactor.h"
#include "thread.h"
#include "tcpclient.h"

static QAtomicInt totalCount = 0;

SubReactor::SubReactor(QObject *parent) : QObject(parent)
{
}

SubReactor::~SubReactor()
{
    qDebug() << "~SubReactor";
}

void SubReactor::onNewConnect(qintptr socketfd)
{
    qDebug() << "Test onNewConnect: " << QThread::currentThreadId();
    TcpClient *tcpSocket = new TcpClient(this);
    if(!tcpSocket->setSocketDescriptor(socketfd)){
        qWarning() << tr("connection failed fd: ") << socketfd
                   << tcpSocket->errorString();
        return;
    }
    connect(tcpSocket, &TcpClient::readyRead, tcpSocket, &TcpClient::onReadyRead);
    connect(tcpSocket, &TcpClient::disconnected, tcpSocket, &TcpClient::deleteLater);
    connect(tcpSocket, &TcpClient::disconnected, this, &SubReactor::onClose, Qt::DirectConnection);
    count++;
    totalCount.fetchAndAddOrdered(1);
    QString str = QString::number(totalCount) + tr(" Client online: ") +
            tcpSocket->getInfo();
    emit message(str);
    emit maxCount(totalCount);
    emit clientCount(totalCount);
    qDebug() << "Thread: " << QThread::currentThreadId()
             << "The current number of clients is: " << count;
}

void SubReactor::onClose()
{
    count--;
    totalCount.fetchAndSubOrdered(1);
    QString str = tr("The client is offline. The current number is: ") +
            QString::number(totalCount);
    emit message(str);
    emit clientCount(totalCount);
}
