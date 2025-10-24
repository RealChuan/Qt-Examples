#pragma once

#include <QStyledItemDelegate>

class ProgressBarDelegate : public QStyledItemDelegate
{
public:
    explicit ProgressBarDelegate(QObject *parent = nullptr);
    ~ProgressBarDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};
