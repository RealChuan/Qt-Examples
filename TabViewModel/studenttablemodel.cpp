#include "studenttablemodel.h"
#include "student.h"

enum Property{ID, NAME, AGE, GENDER, ACHIEVEMENT};

QVariant StuedentTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    Student *stu = m_students[row];
    switch (role) {
    case Qt::CheckStateRole:
        switch (col) {
        case ID: return stu->checked();
        default: break;
        }
        break;
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    case Qt::EditRole: {    //双击为空需添加
        switch (col) {
        case ID:  return stu->Id();
        case NAME: return stu->name();
        case AGE: return stu->age();
        case GENDER: return stu->gender();
        case ACHIEVEMENT: return stu->achievement();
        }
    }
    default: break;
    }
    return QVariant();
}

bool StuedentTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    Student *stu = m_students[row];
    switch (role) {
    case Qt::CheckStateRole:
        switch (col) {
        case ID:
            stu->setchecked(!stu->checked());
            emit dataChanged(index, index);
            return true;
        default: break;
        }
        break;
    case Qt::EditRole:
        switch (col) {
        case ID: stu->setID(quint16(value.toUInt())); break;
        case NAME: stu->setName(value.toString()); break;
        case AGE: stu->setAge(quint16(value.toUInt())); break;
        case GENDER: stu->setGender(value.toString()); break;
        case ACHIEVEMENT:stu->setAchievement(quint16(value.toUInt())); break;
        }
        emit dataChanged(index, index);
        return true;
    default: break;
    }
    return false;
}

QVariant StuedentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const QStringList names = {tr("ID"), tr("NAME"), tr("AGE"), tr("GENDER"), tr("ACHIEVEMENT")};
    if(section < 0 || section >= names.size())
        return QVariant();

    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return names.at(section);
    return QVariant();
}

Qt::ItemFlags StuedentTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsEditable;
    if(index.column() == ID)
        flags |= Qt::ItemIsUserCheckable;
    return flags;
}

void StuedentTableModel::setStudents(const QList<Student *> &students)
{
    beginResetModel();
    m_students = students;
    endResetModel();
}
