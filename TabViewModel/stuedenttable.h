#ifndef STUDENTTABLE_H
#define STUDENTTABLE_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>

#include "student.h"

class StuedentTableModel;
class StudentsTable : public QTableView
{
    Q_OBJECT
public:
    StudentsTable(QWidget *parent = nullptr);

    void setStudents(const QVector<Student> &students);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void insertItem();
    void removeItem();

private:
    void initMenu();

    StuedentTableModel *m_stuModel;
    QMenu *m_menu;
};

#endif // STUDENTTABLE_H
