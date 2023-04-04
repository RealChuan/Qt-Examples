#include "treeview.hpp"

#include <QDebug>
#include <QHeaderView>
#include <QMouseEvent>
#include <QScrollBar>

TreeView::TreeView(QWidget *parent)
    : QTreeView(parent)
{
    setupUI();
    buildConnect();
}

auto TreeView::selectedIndexes() const -> QModelIndexList
{
    QModelIndexList indexList(QTreeView::selectedIndexes());
    QModelIndexList list;
    for (const auto &index : qAsConst(indexList)) {
        QModelIndex i(index.siblingAtColumn(0));
        if (list.contains(i)) {
            continue;
        }
        list.append(i);
    }
    return list;
}

auto TreeView::allIndexes() const -> QModelIndexList
{
    QModelIndexList list;
    QAbstractItemModel *model = this->model();
    if (!model) {
        return list;
    }
    for (int i = 0; i < model->rowCount(); i++) {
        list.append(model->index(i, 0));
    }
    return list;
}

auto TreeView::pageSize() -> int
{
    m_pageSize = height() / 50 + 2;
    return m_pageSize;
}

void TreeView::onCheckAll(bool state)
{
    state ? selectAll() : clearSelection();
}

void TreeView::onResize()
{
    QAbstractItemModel *model = this->model();
    if (!model) {
        return;
    }
    int column = model->columnCount();
    int width = (this->width() - 20) / column;
    for (int i = 0; i < column; i++) {
        setColumnWidth(i, width);
    }
}

void TreeView::onFetchMore(int value)
{
    QAbstractItemModel *model = this->model();
    if (!model) {
        return;
    }
    if (model->rowCount() < m_pageSize || model->rowCount() >= m_currentMaxCount
        || verticalScrollBar()->maximum() != value) {
        return;
    }
    verticalScrollBar()->setEnabled(false);
    emit fetchMore();
    verticalScrollBar()->setEnabled(true);
}

void TreeView::showEvent(QShowEvent *event)
{
    QTreeView::showEvent(event);
    QMetaObject::invokeMethod(this, "onResize", Qt::QueuedConnection);
}

void TreeView::setupUI()
{
    setSortingEnabled(true);
    setDragEnabled(true);
    setAnimated(true);
    setAlternatingRowColors(true);
    setIndentation(7);
    //setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(false);
    //    header()->setVisible(true);
    header()->setHighlightSections(true);
    //header()->setDefaultAlignment(Qt::AlignVCenter);
    //    header()->setSectionResizeMode(QHeaderView::Stretch);
    header()->setStretchLastSection(true);
    setIconSize(QSize(30, 30));
    viewport()->setAttribute(Qt::WA_Hover);

    setStyleSheet("QTreeView{border:none; show-decoration-selected: 1;}"
                  "QTreeView::item{height: 50px;border: 1px solid transparent;}"
                  "QTreeView::branch:hover,"
                  "QTreeView::item:hover{background: #EBF5FF;}"
                  "QTreeView::branch:selected,"
                  "QTreeView::item:selected{background: #b8ceff; color:#222222;}");
}

void TreeView::buildConnect()
{
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TreeView::onFetchMore);
}
