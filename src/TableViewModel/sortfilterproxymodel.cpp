#include "sortfilterproxymodel.h"

#include <QDateTime>

auto SortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const -> bool
{
    QVariant leftData = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);

    if (leftData.userType() == QMetaType::QDateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    } else if(leftData.userType() == QMetaType::Int) {
        return leftData.toInt() > rightData.toInt();
    }
    QString leftString = leftData.toString();
    QString rightString = rightData.toString();
    return QString::localeAwareCompare(leftString, rightString) < 0;
}
