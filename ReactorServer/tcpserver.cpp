#include "tcpserver.h"
#include "thread.h"
#include "subreactor.h"

TcpServer::TcpServer(int num, QObject *parent)
    : QTcpServer(parent)
    , index(0)
{
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<QAtomicInt>("QAtomicInt");

    for(int i=0; i<num; i++){
        Thread *thread = new Thread(this);
        connect(thread, &Thread::message, this, &TcpServer::message);
        connect(thread, &Thread::maxCount, this, &TcpServer::maxCount);
        connect(thread, &Thread::clientCount, this, &TcpServer::clientCount);
        threads.append(thread);
        thread->start();
    }

    if(num <= 0){
        subReactor = new SubReactor(this);
        connect(subReactor, &SubReactor::message, this, &TcpServer::message);
        connect(subReactor, &SubReactor::maxCount, this, &TcpServer::maxCount);
        connect(subReactor, &SubReactor::clientCount, this, &TcpServer::clientCount);
    }
}

TcpServer::~TcpServer()
{
    qDeleteAll(threads);
    threads.clear();
    close();
    qDebug() << "~TcpServer";
}

//-------------------------------------------------
//                            Accepter
//-------------------------------------------------
void TcpServer::incomingConnection(qintptr handle)
{
    Thread *thread = getNextThread();
    if(thread != nullptr)
        emit thread->newConnectHandle(handle);
    else if(subReactor != nullptr)
        subReactor->onNewConnect(handle);
}

Thread *TcpServer::getNextThread()
{
    if(threads.isEmpty())
        return nullptr;
    if(index < 0 || index >= threads.size())
        index = 0;
    Thread* thread = threads.at(index);
    index++;
    return thread;
}
