#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

class ReactorServer : public QObject
{
    Q_OBJECT
public:
    explicit ReactorServer(quint16 port, int threadCount = 4, QObject *parent = nullptr);
    ~ReactorServer();

    void setCallbacks(const ConnectionCallbacks &callbacks); // before start

    void start();
    void stop();
    bool isRunning() const;

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

private:
    class ReactorServerPrivate;
    QScopedPointer<ReactorServerPrivate> d_ptr;
};
