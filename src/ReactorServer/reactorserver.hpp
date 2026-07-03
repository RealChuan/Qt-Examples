#pragma once

#include "connectioncallbacks.hpp"

#include <QObject>

#include <memory>

class ReactorServer : public QObject
{
    Q_OBJECT
public:
    explicit ReactorServer(quint16 port, int threadCount = 4, QObject *parent = nullptr);
    ~ReactorServer();

    void setCallbacks(const ConnectionCallbacks &callbacks); // before start

    void start();
    void stop();
    [[nodiscard]] bool isRunning() const;

signals:
    void message(const QString &msg);
    void clientCountChanged(int count);
    void maxClientCount(int count);

private:
    class ReactorServerPrivate;
    std::unique_ptr<ReactorServerPrivate> d_ptr;
};
