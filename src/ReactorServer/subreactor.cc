#include "subreactor.hpp"
#include "tcpconnection.hpp"

#include <QDebug>

class SubReactor::SubReactorPrivate
{
public:
    explicit SubReactorPrivate(SubReactor *q)
        : q_ptr(q)
    {}

    ~SubReactorPrivate()
    {
        if (connections.isEmpty()) {
            return;
        }

        qDeleteAll(connections);
        connections.clear();
    }

    SubReactor *q_ptr;

    TcpConnectionList connections;
    int connectionCount = 0;
    ConnectionCallbacks callbacks;
};

SubReactor::SubReactor(const ConnectionCallbacks &callbacks, QObject *parent)
    : QObject(parent)
    , d_ptr(new SubReactorPrivate(this))
{
    d_ptr->callbacks = callbacks;
}

SubReactor::~SubReactor() {}

void SubReactor::addConnection(qintptr socketDescriptor)
{
    auto *connection = new TcpConnection(socketDescriptor, d_ptr->callbacks, this);

    if (!connection->isValid()) {
        delete connection;
        emit message(tr("Failed to create connection: %1").arg(connection->errorString()));
        return;
    }

    connect(connection, &TcpConnection::disconnected, this, &SubReactor::onConnectionClosed);

    d_ptr->connections.append(connection);
    d_ptr->connectionCount++;

    emit message(tr("New connection: %1").arg(connection->clientInfo()));
    emit clientConnected();
}

int SubReactor::clientCount() const
{
    return d_ptr->connectionCount;
}

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
