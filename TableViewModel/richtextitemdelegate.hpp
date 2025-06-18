#pragma once

#include <QStyledItemDelegate>

class RichTextItemDelegate : public QStyledItemDelegate
{
public:
    explicit RichTextItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
        -> QSize override;
};
