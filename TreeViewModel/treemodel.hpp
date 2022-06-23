#ifndef TREEMODEL_HPP
#define TREEMODEL_HPP

#include <QAbstractItemModel>

namespace Utils {

void writeAssertLocation(const char *msg);
void dumpBacktrace(int maxdepth);
} // namespace Utils

#define QTC_ASSERT_STRINGIFY_HELPER(x) #x
#define QTC_ASSERT_STRINGIFY(x) QTC_ASSERT_STRINGIFY_HELPER(x)
#define QTC_ASSERT_STRING(cond) \
    Utils::writeAssertLocation("\"" cond "\" in file " __FILE__ \
                               ", line " QTC_ASSERT_STRINGIFY(__LINE__))

// The 'do {...} while (0)' idiom is not used for the main block here to be
// able to use 'break' and 'continue' as 'actions'.

#define QTC_ASSERT(cond, action) \
    if (Q_LIKELY(cond)) { \
    } else { \
        QTC_ASSERT_STRING(#cond); \
        action; \
    } \
    do { \
    } while (0)
#define QTC_CHECK(cond) \
    if (Q_LIKELY(cond)) { \
    } else { \
        QTC_ASSERT_STRING(#cond); \
    } \
    do { \
    } while (0)
#define QTC_GUARD(cond) ((Q_LIKELY(cond)) ? true : (QTC_ASSERT_STRING(#cond), false))

class BaseTreeModel;

class TreeItem
{
public:
    TreeItem();
    virtual ~TreeItem();

    virtual QVariant data(int column, int role) const;
    virtual bool setData(int column, const QVariant &data, int role);
    virtual Qt::ItemFlags flags(int column) const;

    virtual bool hasChildren() const;
    virtual bool canFetchMore() const;
    virtual void fetchMore() {}

    TreeItem *parent() const { return m_parent; }

    void prependChild(TreeItem *item);
    void appendChild(TreeItem *item);
    void insertChild(int pos, TreeItem *item);
    void insertOrderedChild(TreeItem *item,
                            const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);

    void removeChildAt(int pos);
    void removeChildren();
    void sortChildren(const std::function<bool(const TreeItem *, const TreeItem *)> &cmp);
    void update();
    void updateAll();
    void updateColumn(int column);
    void expand();
    void collapse();
    TreeItem *firstChild() const;
    TreeItem *lastChild() const;
    int level() const;

    using const_iterator = QVector<TreeItem *>::const_iterator;
    using value_type = TreeItem *;
    int childCount() const { return m_children.size(); }
    int indexInParent() const;
    TreeItem *childAt(int index) const;
    int indexOf(const TreeItem *item) const;
    const_iterator begin() const { return m_children.begin(); }
    const_iterator end() const { return m_children.end(); }
    QModelIndex index() const;
    QAbstractItemModel *model() const;

    void forSelectedChildren(const std::function<bool(TreeItem *)> &pred) const;
    void forAllChildren(const std::function<void(TreeItem *)> &pred) const;
    TreeItem *findAnyChild(const std::function<bool(TreeItem *)> &pred) const;
    // like findAnyChild() but processes children in exact reverse order
    // (bottom to top, most inner children first)
    TreeItem *reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const;

    // Levels are 1-based: Child at Level 1 is an immediate child.
    void forChildrenAtLevel(int level, const std::function<void(TreeItem *)> &pred) const;
    TreeItem *findChildAtLevel(int level, const std::function<bool(TreeItem *)> &pred) const;

private:
    TreeItem(const TreeItem &) = delete;
    void operator=(const TreeItem &) = delete;

    void clear();
    void removeItemAt(int pos);
    void propagateModel(BaseTreeModel *m);

    TreeItem *m_parent = nullptr;     // Not owned.
    BaseTreeModel *m_model = nullptr; // Not owned.
    QVector<TreeItem *> m_children;   // Owned.
    friend class BaseTreeModel;
};

// A general purpose multi-level model where each item can have its
// own (TreeItem-derived) type.
class BaseTreeModel : public QAbstractItemModel
{
    Q_OBJECT

protected:
    explicit BaseTreeModel(QObject *parent = nullptr);
    explicit BaseTreeModel(TreeItem *root, QObject *parent = nullptr);
    ~BaseTreeModel() override;

    void setHeader(const QStringList &displays);
    void setHeaderToolTip(const QStringList &tips);
    void clear();

    TreeItem *rootItem() const;
    void setRootItem(TreeItem *item);
    TreeItem *itemForIndex(const QModelIndex &) const;
    QModelIndex indexForItem(const TreeItem *needle) const;

    int rowCount(const QModelIndex &idx = QModelIndex()) const override;
    int columnCount(const QModelIndex &idx) const override;

    bool setData(const QModelIndex &idx, const QVariant &data, int role) override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QModelIndex index(int, int, const QModelIndex &idx = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool hasChildren(const QModelIndex &idx) const override;

    bool canFetchMore(const QModelIndex &idx) const override;
    void fetchMore(const QModelIndex &idx) override;

    TreeItem *takeItem(TreeItem *item); // item is not destroyed.
    void destroyItem(TreeItem *item);   // item is destroyed.

signals:
    void requestExpansion(const QModelIndex &);
    void requestCollapse(const QModelIndex &);

protected:
    friend class TreeItem;

    TreeItem *m_root; // Owned.
    QStringList m_header;
    QStringList m_headerToolTip;
    int m_columnCount;
};

#endif // TREEMODEL_HPP
