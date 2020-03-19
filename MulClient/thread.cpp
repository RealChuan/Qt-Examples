#include "thread.h"
#include "tcpclient.h"

Thread::Thread(const QString &ip_, int port_, QObject *parent)
    : QThread(parent)
    , ip(ip_)
    , port(port_)
{

}

Thread::~Thread(){
    if(isRunning()){
        quit();
        wait();
    }
    qDebug() << "~Thread";
}

void Thread::run()
{
    QScopedPointer<TcpClient> client(new TcpClient);
    client->connectToHost(ip, quint16(port));
    if(!client->waitForConnected(3000)){
        qWarning() << tr("connection failed fd: ")
                   << client->errorString();
        return;
    }
//    while(!client->waitForConnected(3000)){
//        client->connectToHost(ip, quint16(port));
//        qWarning() << tr("connection failed fd: ")
//                   << client->errorString();
//    }
    qDebug() << "New Client: " << QThread::currentThreadId();
    connect(this, &Thread::writeToServer, client.data(), &TcpClient::onWrite);
    connect(client.data(), &TcpClient::readyRead,
            client.data(), &TcpClient::onReadyRead, Qt::DirectConnection);
    connect(client.data(), &TcpClient::disconnected, this, &QThread::deleteLater);
    exec();
}
