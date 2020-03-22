#ifndef THREAD3_H
#define THREAD3_H

#include <QObject>

class Test;
class Thread3 : public QObject
{
    Q_OBJECT
public:
    explicit Thread3(QObject *parent = nullptr);
    ~Thread3();

public slots:
    //void doSomeThing(int index = 3);

signals:
    void doSomething(int index = 3);
    void message(const QString&);

private:
    QScopedPointer<Test> test;
    QThread *thread;
};

#endif // THREAD3_H
