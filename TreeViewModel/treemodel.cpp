#include "treemodel.h"
#include "treeitem.h"

#define CHECK_INDEX(index)

/*!
    \class Utils::TreeModel

    \brief The TreeModel class is a convienience base class for models
    to use in a QTreeView.
*/

BaseTreeModel::BaseTreeModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_root(new TreeItem)
{
    m_columnCount = 1;
    m_root->m_model = this;
}

BaseTreeModel::BaseTreeModel(TreeItem *root, QObject *parent)
    : QAbstractItemModel(parent),
      m_root(root)
{
    m_columnCount = 1;
    m_root->propagateModel(this);
}

BaseTreeModel::~BaseTreeModel()
{
    QTC_ASSERT(m_root, return);
    QTC_ASSERT(m_root->m_parent == nullptr, return);
    QTC_ASSERT(m_root->m_model == this, return);
    m_root->m_model = nullptr;
    delete m_root;
}

QModelIndex BaseTreeModel::parent(const QModelIndex &idx) const
{
    CHECK_INDEX(idx);
    if (!idx.isValid())
        return QModelIndex();

    const TreeItem *item = itemForIndex(idx);
    QTC_ASSERT(item, return QModelIndex());
    TreeItem *parent = item->parent();
    if (!parent || parent == m_root)
        return QModelIndex();

    const TreeItem *grandparent = parent->parent();
    if (!grandparent)
        return QModelIndex();

    // This is on the performance-critical path for ItemViewFind.
    const int i = grandparent->m_children.indexOf(parent);
    return createIndex(i, 0, static_cast<void*>(parent));
}

QModelIndex BaseTreeModel::sibling(int row, int column, const QModelIndex &idx) const
{
    const TreeItem *item = itemForIndex(idx);
    QTC_ASSERT(item, return QModelIndex());
    QModelIndex result;
    if (TreeItem *parent = item->parent()) {
        if (TreeItem *sibl = parent->childAt(row))
            result = createIndex(row, column, static_cast<void*>(sibl));
    }
    return result;
}

int BaseTreeModel::rowCount(const QModelIndex &idx) const
{
    CHECK_INDEX(idx);
    if (!idx.isValid())
        return m_root->childCount();
    if (idx.column() > 0)
        return 0;
    const TreeItem *item = itemForIndex(idx);
    return item ? item->childCount() : 0;
}

int BaseTreeModel::columnCount(const QModelIndex &idx) const
{
    CHECK_INDEX(idx);
    if (idx.column() > 0)
        return 0;
    return m_columnCount;
}

bool BaseTreeModel::setData(const QModelIndex &idx, const QVariant &data, int role)
{
    TreeItem *item = itemForIndex(idx);
    bool res = item ? item->setData(idx.column(), data, role) : false;
    if (res)
        emit dataChanged(idx, idx);
    return res;
}

QVariant BaseTreeModel::data(const QModelIndex &idx, int role) const
{
    TreeItem *item = itemForIndex(idx);
    return item ? item->data(idx.column(), role) : QVariant();
}

QVariant BaseTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < m_header.size())
        return m_header.at(section);
    if (role == Qt::ToolTipRole && section < m_headerToolTip.size())
        return m_headerToolTip.at(section);
    return QVariant();
}

bool BaseTreeModel::hasChildren(const QModelIndex &idx) const
{
    if (idx.column() > 0)
        return false;
    TreeItem *item = itemForIndex(idx);
    return !item || item->hasChildren();
}

Qt::ItemFlags BaseTreeModel::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return {};
    TreeItem *item = itemForIndex(idx);
    return item ? item->flags(idx.column())
                : (Qt::ItemIsEnabled|Qt::ItemIsSelectable);
}

bool BaseTreeModel::canFetchMore(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return false;
    TreeItem *item = itemForIndex(idx);
    return item ? item->canFetchMore() : false;
}

void BaseTreeModel::fetchMore(const QModelIndex &idx)
{
    if (!idx.isValid())
        return;
    TreeItem *item = itemForIndex(idx);
    if (item)
        item->fetchMore();
}

TreeItem *BaseTreeModel::rootItem() const
{
    return m_root;
}

void BaseTreeModel::setRootItem(TreeItem *item)
{
    QTC_ASSERT(item, return);
    QTC_ASSERT(item->m_model == nullptr, return);
    QTC_ASSERT(item->m_parent == nullptr, return);
    QTC_ASSERT(item != m_root, return);
    //QTC_CHECK(m_root);

    beginResetModel();
    if (m_root) {
        //QTC_CHECK(m_root->m_parent == nullptr);
        //QTC_CHECK(m_root->m_model == this);
        // needs to be done explicitly before setting the model to 0, otherwise it might lead to a
        // crash inside a view or proxy model, especially if there are selected items
        m_root->removeChildren();
        m_root->m_model = nullptr;
        delete m_root;
    }
    m_root = item;
    item->propagateModel(this);
    endResetModel();
}

void BaseTreeModel::setHeader(const QStringList &displays)
{
    m_header = displays;
    m_columnCount = displays.size();
}

void BaseTreeModel::setHeaderToolTip(const QStringList &tips)
{
    m_headerToolTip = tips;
}

QModelIndex BaseTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    CHECK_INDEX(parent);
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem *item = itemForIndex(parent);
    QTC_ASSERT(item, return QModelIndex());
    if (row >= item->childCount())
        return QModelIndex();
    return createIndex(row, column, static_cast<void*>(item->childAt(row)));
}

TreeItem *BaseTreeModel::itemForIndex(const QModelIndex &idx) const
{
    CHECK_INDEX(idx);
    TreeItem *item = idx.isValid() ? static_cast<TreeItem*>(idx.internalPointer()) : m_root;
    QTC_ASSERT(item, return nullptr);
    QTC_ASSERT(item->m_model == static_cast<const BaseTreeModel *>(this), return nullptr);
    return item;
}

QModelIndex BaseTreeModel::indexForItem(const TreeItem *item) const
{
    QTC_ASSERT(item, return QModelIndex());
    if (item == m_root)
        return QModelIndex();

    TreeItem *p = item->parent();
    QTC_ASSERT(p, return QModelIndex());

    auto mitem = const_cast<TreeItem *>(item);
    int row = p->indexOf(mitem);
    return createIndex(row, 0, mitem);
}

/*!
  Destroys all items in them model except the invisible root item.
*/
void BaseTreeModel::clear()
{
    if (m_root)
        m_root->removeChildren();
}

/*!
   Removes the specified item from the model.

   \note The item is not destroyed, ownership is effectively passed to the caller.
   */

TreeItem *BaseTreeModel::takeItem(TreeItem *item)
{
    QTC_ASSERT(item, return item);
    TreeItem *parent = item->parent();
    QTC_ASSERT(parent, return item);
    int pos = parent->indexOf(item);
    QTC_ASSERT(pos != -1, return item);

    QModelIndex idx = indexForItem(parent);
    beginRemoveRows(idx, pos, pos);
    item->m_parent = nullptr;
    item->m_model = nullptr;
    parent->m_children.removeAt(pos);
    endRemoveRows();
    return item;
}

void BaseTreeModel::destroyItem(TreeItem *item)
{
    delete takeItem(item);
}
