#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

#include <memory>

class SubReactor : public QObject
{
    Q_OBJECT
public:
    explicit SubReactor(const ConnectionCallbacks &callbacks = {}, QObject *parent = nullptr);
    ~SubReactor();

    void addConnection(qintptr socketDescriptor);
    [[nodiscard]] int clientCount() const;

signals:
    void message(const QString &msg);
    void clientConnected();
    void clientDisconnected();

private slots:
    void onConnectionClosed();

private:
    class SubReactorPrivate;
    std::unique_ptr<SubReactorPrivate> d_ptr;
};
