#pragma once

#include <QObject>

#include <memory>

class SqliteTest : public QObject
{
    Q_OBJECT
public:
    explicit SqliteTest(const QString &dbPath = {}, QObject *parent = nullptr);
    ~SqliteTest() override;

    bool insert(const QString &brand, int num);
    bool readLastRecord();

private:
    class SqliteTestPrivate;
    std::unique_ptr<SqliteTestPrivate> d_ptr;
};
