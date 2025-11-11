#pragma once

#include "connectioncallbacks.hpp"

#include <QTcpSocket>

class TcpConnection : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpConnection(qintptr socketDescriptor,
                           const ConnectionCallbacks &callbacks = {},
                           QObject *parent = nullptr);
    ~TcpConnection();

    bool isValid() const;
    QString clientInfo() const;

signals:
    void handleDisconnected();

private slots:
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onDisconnected();

private:
    class TcpConnectionPrivate;
    QScopedPointer<TcpConnectionPrivate> d_ptr;
};

using TcpConnectionList = QList<TcpConnection *>;
