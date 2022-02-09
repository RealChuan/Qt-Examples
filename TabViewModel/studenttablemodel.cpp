#include "studenttablemodel.h"

enum Property { ID, NAME, AGE, GENDER, ACHIEVEMENT, MENUBUTTON, PROCESS, RICHTEXT };

QVariant StuedentTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    Student stu = m_students.at(row);
    switch (role) {
    case Qt::TextAlignmentRole: return Qt::AlignCenter;
    case Qt::CheckStateRole:
        switch (col) {
        case ID: return stu.checked();
        default: break;
        }
        break;
    case Qt::WhatsThisRole:
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole: { //双击为空需添加
        switch (col) {
        case ID: return stu.id();
        case NAME: return stu.name();
        case AGE: return stu.age();
        case GENDER: return stu.gender();
        case ACHIEVEMENT: return stu.achievement();
        case MENUBUTTON: return tr("Detail");
        case PROCESS: return stu.process();
        case RICHTEXT: return stu.richText();
        default: break;
        }
    case Qt::UserRole: {
        switch (col) {
        case MENUBUTTON: return QVariant::fromValue(stu);
        default: break;
        }
    }
    }
    default: break;
    }
    return QVariant();
}

bool StuedentTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();
    int col = index.column();

    Student stu = m_students.at(row);
    switch (role) {
    case Qt::CheckStateRole:
        switch (col) {
        case ID:
            stu.setChecked(!stu.checked());
            emit dataChanged(index, index);
            return true;
        default: break;
        }
        break;
    case Qt::EditRole:
        switch (col) {
        case ID: stu.setId(value.toUInt()); break;
        case NAME: stu.setName(value.toString()); break;
        case AGE: stu.setAge(value.toUInt()); break;
        case GENDER: stu.setGender(value.toString()); break;
        case ACHIEVEMENT: stu.setAchievement(value.toUInt()); break;
        case PROCESS: stu.setProcess(value.toUInt()); break;
        case RICHTEXT: stu.setRichText(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    default: break;
    }
    return false;
}

QVariant StuedentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const QStringList names = {tr("ID"),
                               tr("NAME"),
                               tr("AGE"),
                               tr("GENDER"),
                               tr("ACHIEVEMENT"),
                               tr("DETAILS"),
                               tr("PROCESS"),
                               tr("RICHTEXT")};
    if (section < 0 || section >= names.size()) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return names.at(section);
    }
    return QVariant();
}

Qt::ItemFlags StuedentTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsEditable;
    if (index.column() == ID) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

void StuedentTableModel::setStudents(const QVector<Student> &students)
{
    beginResetModel();
    m_students = students;
    endResetModel();
}
