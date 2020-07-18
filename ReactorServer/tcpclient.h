#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    QString getInfo() const;

public slots:
    void onReadyRead();
};

#endif // TCPSOCKET_H
