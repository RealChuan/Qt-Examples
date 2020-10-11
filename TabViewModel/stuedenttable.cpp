#include "stuedenttable.h"
#include "studenttablemodel.h"
#include "sortfilterproxymodel.h"

#include <QtWidgets>

StudentsTable::StudentsTable(QWidget *parent)
    : QTableView(parent)
    , m_stuModel(new StuedentTableModel(this))
    , m_menu(new QMenu(this))
{
    setShowGrid(true);
    setAlternatingRowColors(true);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(30);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(90);
    horizontalHeader()->setMinimumSectionSize(35);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    setItemDelegateForColumn(3, new EditDelegate(this));

    setSortingEnabled(true);
    SortFilterProxyModel *sortModel = new SortFilterProxyModel(this);
    sortModel->setSourceModel(m_stuModel);
    setModel(sortModel);
    //setModel(m_stuModel);
    initMenu();
}

void StudentsTable::setStudents(const QList<Student *> &students)
{
    m_stuModel->setStudents(students);
}

void StudentsTable::contextMenuEvent(QContextMenuEvent *event)
{
    if(!currentIndex().isValid())
        return;
    m_menu->exec(mapToGlobal(event->pos()));
}

void StudentsTable::initMenu()
{
    m_menu->addAction(tr("insert"), this, &StudentsTable::insertItem);
    m_menu->addAction(tr("remove"), this, &StudentsTable::removeItem);
    m_menu->addAction(tr("rename"), [this] {edit(currentIndex());});
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
