#include "detailsbutton.h"
#include "student.h"

#include <QtWidgets>

DetailsButton::DetailsButton(QWidget *parent)
    : QToolButton(parent)
{
    connect(this, &QToolButton::clicked, this, &DetailsButton::onDetails);
}

void DetailsButton::setStudent(Student *stu)
{
    m_students = stu;
}

void DetailsButton::onDetails()
{
    QString details = tr("This Student id = %1, name = %2, age = %3, "
                         "gender = %4, achievement = %5, details = %6")
            .arg(m_students->Id())
            .arg(m_students->name())
            .arg(m_students->age())
            .arg(m_students->gender())
            .arg(m_students->achievement())
            .arg(m_students->details());

    QDialog dialog;
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
    layout->addWidget(new QLabel(details, this));
    dialog.exec();
}

QWidget *ButtonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    DetailsButton* detailsButton = new DetailsButton(parent);
    return detailsButton;
}

void ButtonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    DetailsButton* detailsButton = qobject_cast<DetailsButton*>(editor);
    detailsButton->setText(index.data(Qt::DisplayRole).toString() + QString::number(index.row()));
    detailsButton->setStudent(index.data(Qt::UserRole).value<Student*>());
}

QWidget *EditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems(QStringList() << tr("MALE") << tr("FEMALE"));
    return comboBox;
}

void EditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void EditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}
