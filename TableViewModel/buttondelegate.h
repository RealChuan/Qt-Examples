#ifndef BUTTONDELEGATE_H
#define BUTTONDELEGATE_H

#include <QStyledItemDelegate>

class ButtonDelegate : public QStyledItemDelegate
{
public:
    explicit ButtonDelegate(QObject *parent = nullptr);
    ~ButtonDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    auto editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) -> bool override;

private:
    QScopedPointer<QStyleOptionButton> m_buttonPtr;
};

#endif // BUTTONDELEGATE_H
