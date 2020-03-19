#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    TcpSocket(QObject *parent = nullptr);
    ~TcpSocket();

    QString getInfo() const;

public slots:
    void onReadyRead();
};

#endif // TCPSOCKET_H
