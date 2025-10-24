#include "thread.h"
#include "tcpclient.h"

#include <QHostAddress>
#include <QAtomicInt>

static QAtomicInt count = 0;

Thread::Thread(qintptr socketDescriptor, QObject *parent)
    : QThread(parent)
    , socketfd(socketDescriptor)
{
}

Thread::~Thread()
{
    if(isRunning()){
        quit();
        wait();
    }
    count.fetchAndSubOrdered(1);
    QString str = tr("The client is offline. The current number is: ") +
                  QString::number(count);
    emit message(str);
    emit clientCount(count);
    qDebug() << str;
    qDebug() << "~Thread";
}

void Thread::run()
{
    QScopedPointer<TcpClient> tcpSocket(new TcpClient);
    if(!tcpSocket->setSocketDescriptor(socketfd)){
        qWarning() << tr("connection failed fd: ") << socketfd
                   << tcpSocket->errorString();
        return;
    }
    qDebug() << "New Client: " << QThread::currentThreadId();
    count.fetchAndAddOrdered(1);
    connect(tcpSocket.data(), &TcpClient::readyRead, tcpSocket.data(), &TcpClient::onReadyRead);
    connect(tcpSocket.data(), &TcpClient::disconnected, this, &Thread::deleteLater);
    QString str = QString::number(count) + tr(" Client online: ") + tcpSocket->getInfo();
    emit message(str);
    emit maxCount(count);
    emit clientCount(count);
    exec();
}
