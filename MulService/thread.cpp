#include "thread.h"
#include "tcpsocket.h"

#include <QHostAddress>
#include <QAtomicInt>

static QAtomicInt count = 0;

Thread::Thread(qintptr socketDescriptor, QObject *parent)
    :QThread(parent)
    ,socketfd(socketDescriptor)
{
    qRegisterMetaType<QAtomicInt>("QAtomicInt");
}

Thread::~Thread()
{
    if(isRunning()){
        quit();
        wait();
    }
    count--;
    QString str = tr("Client Offline Current quantity: ") +
            QString::number(count);
    emit message(str);
    emit clientCount(count);
    qDebug() << str;
    qDebug() << "~Thread";
}

void Thread::run()
{
    QScopedPointer<TcpSocket> tcpSocket(new TcpSocket);
    if(!tcpSocket->setSocketDescriptor(socketfd)){
        qWarning() << tr("connection failed fd: ") << socketfd
                   << tcpSocket->errorString();
        return;
    }
    count++;
    connect(tcpSocket.data(), &TcpSocket::readyRead,
            tcpSocket.data(), &TcpSocket::onReadyRead, Qt::DirectConnection);
    connect(tcpSocket.data(), &TcpSocket::message, this, &Thread::message);
    connect(tcpSocket.data(), &TcpSocket::disconnected, this, &Thread::deleteLater);
    QString str = tr("Client online: ") + tcpSocket->getInfo();
    emit message(str);
    emit maxCount(count);
    emit clientCount(count);
    exec();
}
