#include "studentmodel.h"
#include "treemodel.h"

#include <QHeaderView>

enum Property{ID, NAME, AGE, GENDER, ACHIEVEMENT};

QVariant StudentItem::data(int column, int role) const
{
    switch (role) {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    case Qt::EditRole: {    //双击为空需添加
        switch (column) {
        case ID:            return m_id;
        case NAME:          return m_name;
        case AGE:           return m_age;
        case GENDER:        return m_gender;
        case ACHIEVEMENT:   return m_achievement;
        }
    }
    default: break;
    }
    return QVariant();
}

bool StudentItem::setData(int column, const QVariant &data, int role)
{
    if(role == Qt::EditRole){
        switch (column) {
        case ID:            m_id = quint16(data.toUInt());                break;
        case NAME:          m_name = data.toString();                     break;
        case AGE:           m_age = quint16(data.toUInt());               break;
        case GENDER:        m_gender = data.toString();                   break;
        case ACHIEVEMENT:   m_achievement = quint16(data.toUInt());       break;
        }
        return true;
    }
    return false;
}

Qt::ItemFlags StudentItem::flags(int column) const
{
    Qt::ItemFlags flags = TreeItem::flags(column);
    flags |= Qt::ItemIsEditable;
    return flags;
}


TreeView::TreeView(QWidget *parent) : QTreeView(parent)
{
    setDragEnabled(true);
    setAnimated(true);
    setAlternatingRowColors(true);
    setIndentation(20);
    setHeaderHidden(true);
    //setSelectionMode(QAbstractItemView::SingleSelection);
    //setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(true);
    header()->setVisible(true);
    header()->setHighlightSections(true);
    header()->setDefaultAlignment(Qt::AlignCenter);
    init();
}

void TreeView::updateValueTree(const QVector<ClassPair> &cp)
{
    m_treeModel->clear();
    TreeItem *root = m_treeModel->rootItem();
    for(const ClassPair& cpp: cp){
        StudentItem *item = new StudentItem(cpp.first.Id(), cpp.first.name(),
                                            cpp.first.age(), cpp.first.gender(),
                                            cpp.first.achievement());
        root->appendChild(item);
        for(const StudentItem& stu: cpp.second){
            StudentItem *child = new StudentItem(stu.Id(), stu.name(), stu.age(),
                                                 stu.gender(), stu.achievement());
            item->appendChild(child);
        }
    }
    emit m_treeModel->layoutChanged();
}

void TreeView::init()
{
    m_treeModel = new BaseTreeModel(this);
    m_treeModel->setHeader({ tr("ID"), tr("Name"), tr("AGE"), tr("GENDER"), tr("ACHIEVEMENT") });
    setModel(m_treeModel);
}
