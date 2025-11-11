#include "reactorserver.hpp"
#include "tcpconnection.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

auto main(int argc, char *argv[]) -> int
{
    QCoreApplication app(argc, argv);

    auto getFormattedThreadId = []() {
        return QString("%1").arg(reinterpret_cast<quint64>(QThread::currentThreadId()),
                                 5,
                                 10,
                                 QLatin1Char('0'));
    };

    ReactorServer server(8080, 4);

    // 设置Echo服务器回调
    ConnectionCallbacks callbacks;

    // 然后在回调中使用：
    callbacks.onDataReceived = [getFormattedThreadId](TcpConnection *conn, const QByteArray &data) {
        auto message = QString::fromUtf8(data);
        qDebug() << QString("Thread %1 - Received from %2: %3")
                        .arg(getFormattedThreadId(), conn->clientInfo(), message);
        conn->write(data); // Echo back

        if (message == "::quit" || message == "::shutdown") {
            qInfo() << "Received shutdown command from" << conn->clientInfo();
            QMetaObject::invokeMethod(qApp, &QCoreApplication::quit, Qt::QueuedConnection);
            conn->write("\nServer shutting down...\n");
            return;
        }
    };

    callbacks.onErrorOccurred = [getFormattedThreadId](TcpConnection *conn,
                                                       QAbstractSocket::SocketError socketError) {
        qWarning() << QString("Thread %1 - Client %2, Error: %3, Error String: %4")
                          .arg(getFormattedThreadId(),
                               conn->clientInfo(),
                               QString::number(socketError),
                               conn->errorString());
    };

    callbacks.onConnected = [getFormattedThreadId](TcpConnection *conn) {
        qDebug() << QString("Thread %1 - Client connected: %2")
                        .arg(getFormattedThreadId(), conn->clientInfo());
    };

    callbacks.onDisconnected = [getFormattedThreadId](TcpConnection *conn) {
        qDebug() << QString("Thread %1 - Client disconnected: %2")
                        .arg(getFormattedThreadId(), conn->clientInfo());
    };

    server.setCallbacks(callbacks);

    QObject::connect(&server, &ReactorServer::message, [](const QString &msg) {
        qDebug() << "[SERVER]" << msg;
    });

    server.start();

    return app.exec();
}
