#include "comboboxdelegate.h"

#include <QtWidgets>

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &,
                                        const QModelIndex &) const
{
    static const QStringList items{tr("open"), tr("close")};

    auto comboBox = new QComboBox(parent);
    comboBox->addItems(items);
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto comboBox = qobject_cast<QComboBox *>(editor);
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void ComboBoxDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    auto comboBox = qobject_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}
