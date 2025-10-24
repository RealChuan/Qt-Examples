#include "accepter.h"
#include "tcpserver.h"

#include <QDebug>

Accepter::Accepter(quint16 port_, int num_, QObject *parent)
    : QThread(parent)
    , port(port_)
    , num(num_)
{}

Accepter::~Accepter()
{
    if (isRunning()) {
        quit();
        wait();
    }
    qDebug() << "~Accepter";
}

void Accepter::run()
{
    QScopedPointer<TcpServer> tcpServer(new TcpServer(num));
    connect(tcpServer.data(), &TcpServer::destroyed, this, &Accepter::deleteLater);
    connect(tcpServer.data(), &TcpServer::message, this, &Accepter::message);
    connect(tcpServer.data(), &TcpServer::maxCount, this, &Accepter::maxCount);
    connect(tcpServer.data(), &TcpServer::clientCount, this, &Accepter::clientCount);
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "TcpServer online failure: " << tcpServer->errorString()
                 << QThread::currentThreadId();
        return;
    }
    qDebug() << "TcpServer online: " << QThread::currentThreadId();
    exec();
}
