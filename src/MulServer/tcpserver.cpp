#include "tcpserver.h"
#include "thread.h"

#include <QDebug>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<QAtomicInt>("QAtomicInt");
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
    thread->start();
}
