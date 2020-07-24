#include "treeitem.h"
#include "treemodel.h"

#include <QModelIndex>

//
// TreeItem
//

TreeItem::TreeItem() = default;

TreeItem::~TreeItem()
{
    //QTC_CHECK(m_parent == nullptr);
    //QTC_CHECK(m_model == nullptr);
    removeChildren();
}

TreeItem *TreeItem::childAt(int pos) const
{
    QTC_ASSERT(pos >= 0, return nullptr);
    return pos < childCount() ? *(begin() + pos) : nullptr;
}

int TreeItem::indexOf(const TreeItem *item) const
{
    auto it = std::find(begin(), end(), item);
    return it == end() ? -1 : it - begin();
}

QVariant TreeItem::data(int column, int role) const
{
    Q_UNUSED(column)
    Q_UNUSED(role)
    return QVariant();
}

bool TreeItem::setData(int column, const QVariant &data, int role)
{
    Q_UNUSED(column)
    Q_UNUSED(data)
    Q_UNUSED(role)
    return false;
}

Qt::ItemFlags TreeItem::flags(int column) const
{
    Q_UNUSED(column)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool TreeItem::hasChildren() const
{
    return canFetchMore() || childCount() > 0;
}

bool TreeItem::canFetchMore() const
{
    return false;
}

void TreeItem::prependChild(TreeItem *item)
{
    insertChild(0, item);
}

void TreeItem::appendChild(TreeItem *item)
{
    insertChild(childCount(), item);
}

void TreeItem::insertChild(int pos, TreeItem *item)
{
    //QTC_CHECK(!item->model());
    //QTC_CHECK(!item->parent());
    QTC_ASSERT(0 <= pos && pos <= childCount(), return); // '<=' is intentional.

    if (m_model) {
        QModelIndex idx = index();
        m_model->beginInsertRows(idx, pos, pos);
        item->m_parent = this;
        item->propagateModel(m_model);
        m_children.insert(m_children.begin() + pos, item);
        m_model->endInsertRows();
    } else {
        item->m_parent = this;
        m_children.insert(m_children.begin() + pos, item);
    }
}

void TreeItem::insertOrderedChild(TreeItem *item,
    const std::function<bool (const TreeItem *, const TreeItem *)> &cmp)
{
    auto where = std::lower_bound(begin(), end(), item, cmp);
    insertChild(int(where - begin()), item);
}

void TreeItem::removeChildAt(int pos)
{
    QTC_ASSERT(0 <= pos && pos < m_children.count(), return);

    if (m_model) {
        QModelIndex idx = index();
        m_model->beginRemoveRows(idx, pos, pos);
        removeItemAt(pos);
        m_model->endRemoveRows();
    } else {
        removeItemAt(pos);
    }
}

void TreeItem::removeChildren()
{
    if (childCount() == 0)
        return;
    if (m_model) {
        QModelIndex idx = index();
        m_model->beginRemoveRows(idx, 0, childCount() - 1);
        clear();
        m_model->endRemoveRows();
    } else {
        clear();
    }
}

void TreeItem::sortChildren(const std::function<bool(const TreeItem *, const TreeItem *)> &cmp)
{
    if (m_model) {
        if (const int n = childCount()) {
            QVector<TreeItem *> tmp = m_children;
            std::sort(tmp.begin(), tmp.end(), cmp);
            if (tmp == m_children) {
                // Nothing changed.
            } else {
                QModelIndex idx = index();
                m_model->beginRemoveRows(idx, 0, n - 1);
                m_children.clear();
                m_model->endRemoveRows();
                m_model->beginInsertRows(idx, 0, n - 1);
                tmp.swap(m_children);
                m_model->endInsertRows();
            }
        }
    } else {
        std::sort(m_children.begin(), m_children.end(), cmp);
    }
}

void TreeItem::update()
{
    if (m_model) {
        QModelIndex idx = index();
        emit m_model->dataChanged(idx.sibling(idx.row(), 0),
                                  idx.sibling(idx.row(), m_model->m_columnCount - 1));
    }
}

void TreeItem::updateAll()
{
    if (m_model) {
        QModelIndex idx = index();
        emit m_model->dataChanged(idx, idx.sibling(idx.row(), m_model->m_columnCount - 1));
        for (TreeItem *item : *this)
            item->updateAll();
    }
}

void TreeItem::updateColumn(int column)
{
    if (m_model) {
        QModelIndex idx = index();
        emit m_model->dataChanged(idx.sibling(idx.row(), column), idx.sibling(idx.row(), column));
    }
}

TreeItem *TreeItem::firstChild() const
{
    return childCount() == 0 ? nullptr : *begin();
}

TreeItem *TreeItem::lastChild() const
{
    return childCount() == 0 ? nullptr : *(end() - 1);
}

int TreeItem::level() const
{
    int l = 0;
    for (TreeItem *item = this->parent(); item; item = item->parent())
        ++l;
    return l;
}

int TreeItem::indexInParent() const
{
    return m_parent ? m_parent->indexOf(this) : -1;
}

QModelIndex TreeItem::index() const
{
    QTC_ASSERT(m_model, return QModelIndex());
    return m_model->indexForItem(this);
}

QAbstractItemModel *TreeItem::model() const
{
    return m_model;
}

void TreeItem::forAllChildren(const std::function<void (TreeItem *)> &pred) const
{
    for (TreeItem *item : *this) {
        pred(item);
        item->forAllChildren(pred);
    }
}

void TreeItem::forSelectedChildren(const std::function<bool (TreeItem *)> &pred) const
{
    for (TreeItem *item : *this) {
        if (pred(item))
            item->forSelectedChildren(pred);
    }
}

void TreeItem::forChildrenAtLevel(int level, const std::function<void(TreeItem *)> &pred) const
{
    QTC_ASSERT(level > 0, return);
    if (level == 1) {
        for (TreeItem *item : *this)
            pred(item);
    } else {
        for (TreeItem *item : *this)
            item->forChildrenAtLevel(level - 1, pred);
    }
}

TreeItem *TreeItem::findChildAtLevel(int level, const std::function<bool(TreeItem *)> &pred) const
{
    QTC_ASSERT(level > 0, return nullptr);
    if (level == 1) {
        for (TreeItem *item : *this)
            if (pred(item))
                return item;
    } else {
        for (TreeItem *item : *this) {
            if (auto found = item->findChildAtLevel(level - 1, pred))
                return found;
        }
    }
    return nullptr;
}

TreeItem *TreeItem::findAnyChild(const std::function<bool(TreeItem *)> &pred) const
{
    for (TreeItem *item : *this) {
        if (pred(item))
            return item;
        if (TreeItem *found = item->findAnyChild(pred))
            return found;
    }
    return nullptr;
}

TreeItem *TreeItem::reverseFindAnyChild(const std::function<bool (TreeItem *)> &pred) const
{
    auto end = m_children.rend();
    for (auto it = m_children.rbegin(); it != end; ++it) {
        if (TreeItem *found = (*it)->reverseFindAnyChild(pred))
            return found;
        if (pred(*it))
            return *it;
    }
    return nullptr;
}

void TreeItem::clear()
{
    while (childCount() != 0) {
        TreeItem *item = m_children.takeLast();
        item->m_model = nullptr;
        item->m_parent = nullptr;
        delete item;
    }
}

void TreeItem::removeItemAt(int pos)
{
    TreeItem *item = m_children.at(pos);
    item->m_model = nullptr;
    item->m_parent = nullptr;
    delete item;
    m_children.removeAt(pos);
}

void TreeItem::expand()
{
    QTC_ASSERT(m_model, return);
    emit m_model->requestExpansion(index());
}

void TreeItem::collapse()
{
    QTC_ASSERT(m_model, return);
    emit m_model->requestCollapse(index());
}

void TreeItem::propagateModel(BaseTreeModel *m)
{
    QTC_ASSERT(m, return);
    QTC_ASSERT(m_model == nullptr || m_model == m, return);
    if (m && !m_model) {
        m_model = m;
        for (TreeItem *item : *this)
            item->propagateModel(m);
    }
}
