#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QTcpSocket>

class TcpClient : public QTcpSocket
{
    Q_OBJECT
public:
    TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    QString getInfo() const;

signals:
    void message(QString);

public slots:
    void onWrite(const QByteArray&);
    void onReadyRead();
};

#endif // TCPCLIENT_H
