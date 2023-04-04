#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortFilterProxyModel : public QSortFilterProxyModel
{
public:
    explicit SortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {}

protected:
    [[nodiscard]] auto lessThan(const QModelIndex &left, const QModelIndex &right) const
        -> bool override;
};

#endif // SORTFILTERPROXYMODEL_H
