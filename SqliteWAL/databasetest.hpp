#pragma once

#include <QObject>

class DataBaseTest : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseTest(QObject *parent = nullptr);
    ~DataBaseTest();

    bool insert(const QString &brand, int num);

private:
    class DataBaseTestPrivate;
    QScopedPointer<DataBaseTestPrivate> d_ptr;
};
