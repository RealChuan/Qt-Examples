#include "stuedenttable.h"
#include "buttondelegate.h"
#include "comboboxdelegate.h"
#include "progressbardelegate.h"
#include "richtextitemdelegate.hpp"
#include "sortfilterproxymodel.h"
#include "studenttablemodel.h"

#include <QtWidgets>

StudentsTable::StudentsTable(QWidget *parent)
    : QTableView(parent)
    , m_stuModel(new StuedentTableModel(this))
    , m_menu(new QMenu(this))
{
    SortFilterProxyModel *sortModel = new SortFilterProxyModel(this);
    sortModel->setSourceModel(m_stuModel);
    setModel(sortModel);
    //setModel(m_stuModel);

    setShowGrid(true);
    setWordWrap(false);
    setAlternatingRowColors(true);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(30);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(90);
    horizontalHeader()->setMinimumSectionSize(35);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setItemDelegateForColumn(3, new ComboBoxDelegate(this));
    setItemDelegateForColumn(5, new ButtonDelegate(this));
    setItemDelegateForColumn(6, new ProgressBarDelegate(this));
    setItemDelegateForColumn(7, new RichTextItemDelegate(this));
    setSortingEnabled(true);

    initMenu();
}

void StudentsTable::setStudents(const QVector<Student> &students)
{
    m_stuModel->setStudents(students);
}

void StudentsTable::contextMenuEvent(QContextMenuEvent *event)
{
    if (!currentIndex().isValid())
        return;
    m_menu->exec(mapToGlobal(event->pos()));
}

void StudentsTable::initMenu()
{
    m_menu->addAction(tr("insert"), this, &StudentsTable::insertItem);
    m_menu->addAction(tr("remove"), this, &StudentsTable::removeItem);
    m_menu->addAction(tr("rename"), this, [this] { edit(currentIndex().siblingAtColumn(0)); });
}
