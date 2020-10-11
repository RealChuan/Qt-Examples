#ifndef STUDENTTABLE_H
#define STUDENTTABLE_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>

class Student;
class StuedentTableModel;
class StudentsTable : public QTableView
{
    Q_OBJECT
public:
    StudentsTable(QWidget *parent = nullptr);

    void setStudents(const QList<Student*>& students);

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void insertItem();
    void removeItem();

private:
    void initMenu();

    StuedentTableModel *m_stuModel;
    QMenu *m_menu;
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

#endif // STUDENTTABLE_H
