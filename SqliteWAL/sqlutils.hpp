#ifndef SQLUTILS_HPP
#define SQLUTILS_HPP

#include <QObject>

class SqlUtils : public QObject
{
public:
    SqlUtils(QObject *parent = nullptr);
    ~SqlUtils();

    void insert(const QString &brand, int num);

private:
    class SqlUtilsPrivate;
    QScopedPointer<SqlUtilsPrivate> d_ptr;
};

#endif // SQLUTILS_HPP
