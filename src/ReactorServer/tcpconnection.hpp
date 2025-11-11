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

public slots:
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private slots:
    void handleDisconnected();

private:
    class TcpConnectionPrivate;
    QScopedPointer<TcpConnectionPrivate> d_ptr;
};

using TcpConnectionList = QList<TcpConnection *>;
