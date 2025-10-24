#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboBoxDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    auto createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const -> QWidget * Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // COMBOBOXDELEGATE_H
