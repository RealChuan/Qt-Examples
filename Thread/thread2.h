#ifndef THREAD2_H
#define THREAD2_H

#include <QThread>

class Thread2 : public QThread
{
    Q_OBJECT
public:
    Thread2(QObject *parent = nullptr);
    ~Thread2() override;

signals:
    void doSomeThing(int index = 2);
    void message(const QString&);

public:
    void run() override;
};

#endif // THREAD2_H
