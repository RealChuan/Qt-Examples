#include "tcpconnection.hpp"

#include <QDebug>

class TcpConnection::TcpConnectionPrivate
{
public:
    explicit TcpConnectionPrivate(TcpConnection *q)
        : q_ptr(q)
    {}

    TcpConnection *q_ptr;

    bool valid = false;
    ConnectionCallbacks callbacks;
};

TcpConnection::TcpConnection(qintptr socketDescriptor,
                             const ConnectionCallbacks &callbacks,
                             QObject *parent)
    : QTcpSocket(parent)
    , d_ptr(new TcpConnectionPrivate(this))
{
    if (!setSocketDescriptor(socketDescriptor)) {
        d_ptr->valid = false;
        return;
    }

    d_ptr->valid = true;
    d_ptr->callbacks = callbacks;
    connect(this, &TcpConnection::readyRead, this, &TcpConnection::onReadyRead);
    connect(this, &TcpConnection::errorOccurred, this, &TcpConnection::onErrorOccurred);
    connect(this, &TcpConnection::disconnected, this, &TcpConnection::onDisconnected);

    if (d_ptr->callbacks.onConnected) {
        d_ptr->callbacks.onConnected(this);
    }
}

TcpConnection::~TcpConnection()
{
    if (state() != QAbstractSocket::UnconnectedState) {
        disconnectFromHost();
    }
    qDebug() << "~TcpConnection";
}

bool TcpConnection::isValid() const
{
    return d_ptr->valid;
}

QString TcpConnection::clientInfo() const
{
    return QString("%1:%2").arg(peerAddress().toString(), QString::number(peerPort()));
}

void TcpConnection::onReadyRead()
{
    auto data = readAll();

    if (d_ptr->callbacks.onDataReceived) {
        d_ptr->callbacks.onDataReceived(this, data);
    }
}

void TcpConnection::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    if (d_ptr->callbacks.onErrorOccurred) {
        d_ptr->callbacks.onErrorOccurred(this, socketError);
    }
}

void TcpConnection::onDisconnected()
{
    if (d_ptr->callbacks.onDisconnected) {
        d_ptr->callbacks.onDisconnected(this);
    }

    emit handleDisconnected();
}
