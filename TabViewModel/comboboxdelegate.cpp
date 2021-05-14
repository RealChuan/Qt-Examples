#include "comboboxdelegate.h"
#include "student.h"

#include <QtWidgets>

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &,
                                        const QModelIndex &) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems(QStringList() << tr("MALE") << tr("FEMALE"));
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void ComboBoxDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}
