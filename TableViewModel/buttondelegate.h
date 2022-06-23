#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QStyledItemDelegate>

class ButtonDelegate : public QStyledItemDelegate
{
public:
    ButtonDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QScopedPointer<QStyleOptionButton> m_buttonPtr;
};

#endif // BUTTONDELEGATE_H
