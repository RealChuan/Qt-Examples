#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

class SubReactor : public QObject
{
    Q_OBJECT
public:
    explicit SubReactor(const ConnectionCallbacks &callbacks = {}, QObject *parent = nullptr);
    ~SubReactor();

    void addConnection(qintptr socketDescriptor);
    int clientCount() const;

signals:
    void message(const QString &msg);
    void clientConnected();
    void clientDisconnected();

private slots:
    void onConnectionClosed();

private:
    class SubReactorPrivate;
    QScopedPointer<SubReactorPrivate> d_ptr;
};
