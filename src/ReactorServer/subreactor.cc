#include "subreactor.hpp"
#include "tcpconnection.hpp"

#include <QDebug>

using namespace Qt::StringLiterals;

class SubReactor::SubReactorPrivate
{
public:
    explicit SubReactorPrivate(SubReactor *q) : q_ptr(q) {}

    // No manual deletion needed:
    // - Normally closed connections are removed by onConnectionClosed() → deleteLater()
    // - Remaining connections are QObjects; Qt parent ownership handles cleanup
    // - qDeleteAll here would double-free connections already queued for deleteLater()

    SubReactor *q_ptr;

    TcpConnectionList connections;
    int connectionCount = 0;
    ConnectionCallbacks callbacks;
};

SubReactor::SubReactor(const ConnectionCallbacks &callbacks, QObject *parent)
    : QObject(parent), d_ptr(std::make_unique<SubReactorPrivate>(this))
{ d_ptr->callbacks = callbacks; }

SubReactor::~SubReactor() {}

void SubReactor::addConnection(qintptr socketDescriptor)
{
    auto result = TcpConnection::create(socketDescriptor, d_ptr->callbacks);

    if (!result.has_value()) {
        qCritical() << u"Failed to create connection:"_s << result.error();
        return;
    }

    // Transfer ownership from unique_ptr to Qt event loop (deleteLater)
    auto *connection = result->release();
    connect(connection, &TcpConnection::handleDisconnected, this, &SubReactor::onConnectionClosed);

    d_ptr->connections.append(connection);
    d_ptr->connectionCount++;

    emit message(tr("New connection: %1").arg(connection->clientInfo()));
    emit clientConnected();
}

int SubReactor::clientCount() const
{ return d_ptr->connectionCount; }

void SubReactor::onConnectionClosed()
{
    auto *connection = qobject_cast<TcpConnection *>(sender());
    if (connection) {
        d_ptr->connections.removeAll(connection);
        d_ptr->connectionCount--;
        connection->deleteLater();
        emit message(tr("Connection closed:%1").arg(connection->clientInfo()));
        emit clientDisconnected();
    }
}
