#include "testtable.h"

#include <QtWidgets>

enum Property{ID, NAME, AGE, GENDER, ACHIEVEMENT};

QVariant StuedentTableModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if(row < 0 || row >= m_students.size())
        return QVariant();

    Student *stu = m_students[row];
    switch (role) {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole: {
        switch (col) {
        case ID:            return stu->Id();
        case NAME:          return stu->name();
        case AGE:           return stu->age();
        case GENDER:        return stu->gender();
        case ACHIEVEMENT:   return stu->achievement();
        }
    }
    default: break;
    }
    return QVariant();
}

bool StuedentTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();
    if(row < 0 || row >= m_students.size())
        return false;

    Student *stu = m_students[row];
    if(role == Qt::EditRole){
        switch (col) {
        case ID:            stu->setID(quint16(value.toUInt()));                break;
        case NAME:          stu->setName(value.toString());                     break;
        case AGE:           stu->setAge(quint16(value.toUInt()));               break;
        case GENDER:        stu->setGender(value.toString());                   break;
        case ACHIEVEMENT:   stu->setAchievement(quint16(value.toUInt()));       break;
        }
        emit dataChanged(index, index);
        return true;
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
    return flags;
}

void StuedentTableModel::setStudents(const QList<Student *> &students)
{
    beginResetModel();
    m_students = students;
    endResetModel();
}

StudentsTable::StudentsTable(QWidget *parent)
    :QTableView(parent)
{
    setAlternatingRowColors(true);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(30);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(90);
    horizontalHeader()->setMinimumSectionSize(35);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setItemDelegateForColumn(3, new EditDelegate(this));

    stuModel = new StuedentTableModel(this);
    setModel(stuModel);
}

void StudentsTable::setStudents(const QList<Student *> &students)
{
    stuModel->setStudents(students);
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
