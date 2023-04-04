#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() override;

    [[nodiscard]] auto getInfo() const -> QString;

public slots:
    void onWrite(const QByteArray &);
    void onReadyRead();
};

#endif // TCPCLIENT_H
