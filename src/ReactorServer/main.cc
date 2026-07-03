#include "reactorserver.hpp"
#include "tcpconnection.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include <atomic>

using namespace Qt::StringLiterals;

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication app(argc, argv);

    const auto getFormattedThreadId = []() {
        return u"%1"_s.arg(
            reinterpret_cast<quint64>(QThread::currentThreadId()), 5, 10, QLatin1Char('0'));
    };

    // Active connection count — updated synchronously in SubReactor callbacks.
    // Must use callback-based counting (not signal-synced) because the
    // clientCountChanged signal chain crosses thread boundaries via
    // QueuedConnection (AccepterThread → main thread), causing stale reads
    // when ::stats is queried immediately after a connect/disconnect.
    static std::atomic<int> activeConnectionCount{0};

    ReactorServer server(8080, 4);

    // 设置Echo服务器回调
    ConnectionCallbacks callbacks;

    callbacks.onDataReceived = [getFormattedThreadId](TcpConnection *conn, const QByteArray &data) {
        auto message = QString::fromUtf8(data);
        qDebug() << u"Thread %1 - Received from %2: %3"_s.arg(
            getFormattedThreadId(), conn->clientInfo(), message);

        // Protocol commands — return without echo
        if (message == u"::quit"_s || message == u"::shutdown"_s) {
            qInfo() << "Received shutdown command from" << conn->clientInfo();
            QMetaObject::invokeMethod(qApp, &QCoreApplication::quit, Qt::QueuedConnection);
            conn->write("\nServer shutting down...\n");
            return;
        }
        if (message == u"::info"_s) {
            conn->write(u"thread:%1\n"_s.arg(getFormattedThreadId()).toUtf8());
            return;
        }
        if (message == u"::stats"_s) {
            conn->write(u"active:%1\n"_s.arg(activeConnectionCount.load()).toUtf8());
            return;
        }

        conn->write(data); // Echo back
    };

    callbacks.onErrorOccurred
        = [getFormattedThreadId](TcpConnection *conn, QAbstractSocket::SocketError socketError) {
              qWarning() << u"Thread %1 - Client %2, Error: %3, Error String: %4"_s.arg(
                  getFormattedThreadId(),
                  conn->clientInfo(),
                  QString::number(socketError),
                  conn->errorString());
          };

    callbacks.onConnected = [getFormattedThreadId](TcpConnection *conn) {
        activeConnectionCount.fetch_add(1);
        qDebug() << u"Thread %1 - Client connected: %2 (active: %3)"_s.arg(
            getFormattedThreadId(),
            conn->clientInfo(),
            QString::number(activeConnectionCount.load()));
    };

    callbacks.onDisconnected = [getFormattedThreadId](TcpConnection *conn) {
        activeConnectionCount.fetch_sub(1);
        qDebug() << u"Thread %1 - Client disconnected: %2 (active: %3)"_s.arg(
            getFormattedThreadId(),
            conn->clientInfo(),
            QString::number(activeConnectionCount.load()));
    };

    server.setCallbacks(callbacks);

    QObject::connect(&server, &ReactorServer::message, [](const QString &msg) {
        qDebug() << "[SERVER]" << msg;
    });

    server.start();

    return app.exec();
}
