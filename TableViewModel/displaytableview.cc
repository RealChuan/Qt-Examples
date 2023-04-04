#include "displaytableview.hpp"
#include "buttondelegate.h"
#include "comboboxdelegate.h"
#include "displaytablemodel.hpp"
#include "progressbardelegate.h"
#include "richtextitemdelegate.hpp"
#include "sortfilterproxymodel.h"
#include "stardelegate.h"

#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>

class DisplayTableView::DisplayTableViewPrivate
{
public:
    explicit DisplayTableViewPrivate(DisplayTableView *parent)
        : q_ptr(parent)
    {
        model = new DisplayTableModel(q_ptr);

        menu = new QMenu(q_ptr);
        menu->addAction(QObject::tr("Insert"), q_ptr, &DisplayTableView::insertItem);
        menu->addAction(QObject::tr("Remove"), q_ptr, &DisplayTableView::removeItem);
        menu->addAction(QObject::tr("Rename"), q_ptr, [this] {
            q_ptr->edit(q_ptr->currentIndex().siblingAtColumn(DisplayTableModel::TITLE));
        });
    }

    DisplayTableView *q_ptr;

    DisplayTableModel *model;
    QMenu *menu;
};

DisplayTableView::DisplayTableView(QWidget *parent)
    : QTableView(parent)
    , d_ptr(new DisplayTableViewPrivate(this))
{
    auto sortModel = new SortFilterProxyModel(this);
    sortModel->setSourceModel(d_ptr->model);
    setModel(sortModel);

    setShowGrid(true);
    setWordWrap(false);
    setAlternatingRowColors(true);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(35);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setDefaultSectionSize(120);
    horizontalHeader()->setMinimumSectionSize(60);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setSortingEnabled(true);
    setIconSize(QSize(20, 20));

    setItemDelegateForColumn(DisplayTableModel::STATE, new ComboBoxDelegate(this));
    setItemDelegateForColumn(DisplayTableModel::PROCESS, new ProgressBarDelegate(this));
    setItemDelegateForColumn(DisplayTableModel::RATING, new StarDelegate(this));
    setItemDelegateForColumn(DisplayTableModel::RICHTEXT, new RichTextItemDelegate(this));
    setItemDelegateForColumn(DisplayTableModel::DETAILS, new ButtonDelegate(this));
}

DisplayTableView::~DisplayTableView() = default;

void DisplayTableView::setDatas(const QVector<DisplayInfo> &datas)
{
    d_ptr->model->setDatas(datas);
}

void DisplayTableView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!currentIndex().isValid()) {
        return;
    }
    d_ptr->menu->exec(mapToGlobal(event->pos()));
}
