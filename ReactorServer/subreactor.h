#ifndef SUBREACTOR_H
#define SUBREACTOR_H

#include <QObject>

class SubReactor : public QObject
{
    Q_OBJECT
public:
    explicit SubReactor(QObject *parent = nullptr);
    ~SubReactor() override;

signals:
    void message(const QString&);
    void maxCount(QAtomicInt);
    void clientCount(QAtomicInt);

public slots:
    void onNewConnect(qintptr socketfd);

private slots:
    void onClose();

private:
    QAtomicInt count;
};

#endif // SUBREACTOR_H
