#ifndef TEST_H
#define TEST_H

#include <QObject>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);
    ~Test();

signals:
    void message(const QString&);

public slots:
    void doSomeThing(int);

private:
    int count;
};

#endif // TEST_H
