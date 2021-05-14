#ifndef STUDENTTABLEMODEL_H
#define STUDENTTABLEMODEL_H

#include <QAbstractTableModel>

struct Student;
class StuedentTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit StuedentTableModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {}
    int rowCount(const QModelIndex & = QModelIndex()) const { return m_students.count(); }
    int columnCount(const QModelIndex & = QModelIndex()) const { return 7; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setStudents(const QList<Student *> &students);

private:
    QList<Student *> m_students;
};

#endif // STUDENTTABLEMODEL_H
