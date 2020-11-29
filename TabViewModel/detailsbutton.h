#ifndef DETAILSBUTTON_H
#define DETAILSBUTTON_H

#include <QStyledItemDelegate>
#include <QToolButton>

class Student;
class DetailsButton : public QToolButton
{
    Q_OBJECT
public:
    DetailsButton(QWidget* parent = nullptr);

    void setStudent(Student* stu);

private slots:
    void onDetails();

private:
    Student* m_students;
};

class ButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ButtonDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor,const QModelIndex& index) const Q_DECL_OVERRIDE;
};

class EditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit EditDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

protected:
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor,const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const  QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // DETAILSBUTTON_H
