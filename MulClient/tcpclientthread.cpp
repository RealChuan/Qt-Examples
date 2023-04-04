#include "tcpclientthread.h"
#include "tcpclient.h"

class TcpClientThreadPrivate{
public:
    explicit TcpClientThreadPrivate(QThread *parent = nullptr)
        : owner(parent){}
    QThread *owner;
    QString ip = "127.0.0.1";
    quint16 port = 65533;
    int index = 0;
};

TcpClientThread::TcpClientThread(const QString &ip,
                                 const quint16 port,
                                 const int index,
                                 QObject *parent)
    : QThread(parent)
    , d(new TcpClientThreadPrivate)
{
    d->ip = ip;
    d->port = port;
    d->index = index;
}

TcpClientThread::~TcpClientThread(){
    if(isRunning()){
        quit();
        wait();
    }
    qDebug() << "~Thread" << d->index;
}

void TcpClientThread::removeMyself()
{
    emit quitThread(d->index);
}

void TcpClientThread::run()
{
    QScopedPointer<TcpClient> client(new TcpClient);
    client->connectToHost(d->ip, d->port);
    if(!client->waitForConnected(1000)){
        qWarning() << tr("connection failed fd: ")
                   << client->errorString();
        emit quitThread(d->index);
        return;
    }
    //    while(!client->waitForConnected(3000)){
    //        client->connectToHost(ip, quint16(port));
    //        qWarning() << tr("connection failed fd: ")
    //                   << client->errorString();
    //    }
    qDebug() << "New Client: " << d->index << QThread::currentThreadId();
    connect(this, &TcpClientThread::writeToServer, client.data(), &TcpClient::onWrite);
    connect(client.data(), &TcpClient::readyRead,
            client.data(), &TcpClient::onReadyRead, Qt::DirectConnection);
    connect(client.data(), &TcpClient::disconnected, this, &TcpClientThread::removeMyself);
    exec();
}
