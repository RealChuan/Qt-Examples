#ifndef ACCEPTER_H
#define ACCEPTER_H

#include <QThread>

class Accepter : public QThread
{
    Q_OBJECT
public:
    explicit Accepter(quint16, QObject *parent = nullptr);
    ~Accepter() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

protected:
    void run() override;

private:
    quint16 port;
};

#endif // ACCEPTER_H
