#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() override;

    [[nodiscard]] auto getInfo() const -> QString;

public slots:
    void onReadyRead();
};

#endif // TCPSOCKET_H
