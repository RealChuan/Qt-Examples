#ifndef RICHTEXTITEMDELEGATE_HPP
#define RICHTEXTITEMDELEGATE_HPP

#include <QStyledItemDelegate>
#include <QtCore/qglobal.h>

class RichTextItemDelegate : public QStyledItemDelegate
{
public:
    explicit RichTextItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    [[nodiscard]] auto sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
        -> QSize override;

    Qt::TextElideMode textElideMode = Qt::ElideRight;
};

#endif // RICHTEXTITEMDELEGATE_HPP
