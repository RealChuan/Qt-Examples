#pragma once

#include "connectioncallbacks.hpp"

#include <QTcpSocket>

#include <expected>
#include <memory>

class TcpConnection : public QTcpSocket
{
    Q_OBJECT
public:
    /**
     * @brief Factory method — the only way to create a valid TcpConnection.
     * Returns an error string on failure instead of a silently-broken object.
     */
    [[nodiscard]] static auto create(qintptr socketDescriptor,
                                     const ConnectionCallbacks &callbacks = {},
                                     QObject *parent = nullptr)
        -> std::expected<std::unique_ptr<TcpConnection>, QString>;

    ~TcpConnection();

    [[nodiscard]] QString clientInfo() const;

signals:
    void handleDisconnected();

private slots:
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onDisconnected();

private:
    // Private constructor — use create() factory instead
    explicit TcpConnection(qintptr socketDescriptor,
                           const ConnectionCallbacks &callbacks,
                           QObject *parent);

    class TcpConnectionPrivate;
    std::unique_ptr<TcpConnectionPrivate> d_ptr;
};

using TcpConnectionList = QList<TcpConnection *>;
