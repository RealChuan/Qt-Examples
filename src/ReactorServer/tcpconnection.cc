#include "tcpconnection.hpp"

#include <QDebug>

using namespace Qt::StringLiterals;

class TcpConnection::TcpConnectionPrivate
{
public:
    explicit TcpConnectionPrivate(TcpConnection *q) : q_ptr(q) {}

    TcpConnection *q_ptr;

    ConnectionCallbacks callbacks;
};

auto TcpConnection::create(qintptr socketDescriptor,
                           const ConnectionCallbacks &callbacks,
                           QObject *parent)
    -> std::expected<std::unique_ptr<TcpConnection>, QString>
{
    // Cannot use make_unique with private constructor; use new then wrap.
    auto *raw = new TcpConnection(socketDescriptor, callbacks, parent);

    if (!raw->setSocketDescriptor(socketDescriptor)) {
        auto error = raw->errorString();
        delete raw;
        return std::unexpected(error);
    }

    // Wire up signals only after socket descriptor is valid
    connect(raw, &TcpConnection::readyRead, raw, &TcpConnection::onReadyRead);
    connect(raw, &TcpConnection::errorOccurred, raw, &TcpConnection::onErrorOccurred);
    connect(raw, &TcpConnection::disconnected, raw, &TcpConnection::onDisconnected);

    raw->d_ptr->callbacks = callbacks;

    // Wrap in unique_ptr BEFORE invoking callback — ensures RAII protection
    // if the callback triggers a signal chain that could destroy the object
    auto ptr = std::unique_ptr<TcpConnection>(raw);

    if (ptr->d_ptr->callbacks.onConnected) {
        ptr->d_ptr->callbacks.onConnected(ptr.get());
    }

    return ptr;
}

TcpConnection::TcpConnection(qintptr socketDescriptor,
                             const ConnectionCallbacks &callbacks,
                             QObject *parent)
    : QTcpSocket(parent), d_ptr(std::make_unique<TcpConnectionPrivate>(this))
{
    Q_UNUSED(socketDescriptor)
    Q_UNUSED(callbacks)
    // Socket descriptor and callbacks are set in create() after validation
}

TcpConnection::~TcpConnection()
{
    if (state() != QAbstractSocket::UnconnectedState) {
        disconnectFromHost();
    }
}

QString TcpConnection::clientInfo() const
{ return u"%1:%2"_s.arg(peerAddress().toString(), QString::number(peerPort())); }

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
    // Emit handleDisconnected FIRST so SubReactor decrements its count
    // before the user callback runs — ensures consistent state in callback
    emit handleDisconnected();

    if (d_ptr->callbacks.onDisconnected) {
        d_ptr->callbacks.onDisconnected(this);
    }
}
