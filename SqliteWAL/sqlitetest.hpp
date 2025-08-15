#pragma once

#include <QObject>

class SqliteTest : public QObject
{
    Q_OBJECT
public:
    explicit SqliteTest(QObject *parent = nullptr);
    ~SqliteTest();

    bool insert(const QString &brand, int num);
    bool readLastRecord();

private:
    class SqliteTestPrivate;
    QScopedPointer<SqliteTestPrivate> d_ptr;
};
