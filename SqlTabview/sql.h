#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QSqlError>

class QSqlQuery;
class Sql : public QObject
{
    Q_OBJECT
public:
    explicit Sql(QObject *parent = nullptr);

    static bool searchSQLite(QWidget* parent = nullptr);
    QSqlError createOrOpenSqlite();

signals:

private:
    void addStudent(QSqlQuery&, const QString&, int, const QString&, int);

};

#endif // SQL_H
