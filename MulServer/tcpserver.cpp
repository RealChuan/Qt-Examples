#include "tcpserver.h"
#include "thread.h"

TcpServer::TcpServer(QObject *parent)
    :QTcpServer(parent)
{

}

TcpServer::~TcpServer()
{
    close();
    qDebug() << "~TcpServer";
}

void TcpServer::incomingConnection(qintptr handle)
{
    Thread *thread = new Thread(handle, this);
    connect(thread, &Thread::message, this, &TcpServer::message);
    connect(thread, &Thread::maxCount, this, &TcpServer::maxCount);
    connect(thread, &Thread::clientCount, this, &TcpServer::clientCount);
    //connect(thread, &Thread::finished, thread, &Thread::deleteLater);
    thread->start();
}
