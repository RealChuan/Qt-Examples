#ifndef STUDENTTABLEMODEL_H
#define STUDENTTABLEMODEL_H

#include <QAbstractTableModel>

#include "student.h"

class StuedentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StuedentTableModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {}
    int rowCount(const QModelIndex & = QModelIndex()) const { return m_students.size(); }
    int columnCount(const QModelIndex & = QModelIndex()) const { return 8; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setStudents(const QVector<Student> &students);

private:
    QVector<Student> m_students;
};

#endif // STUDENTTABLEMODEL_H
