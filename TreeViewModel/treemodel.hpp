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

    [[nodiscard]] virtual auto data(int column, int role) const -> QVariant;
    virtual auto setData(int column, const QVariant &data, int role) -> bool;
    [[nodiscard]] virtual Qt::ItemFlags flags(int column) const;

    [[nodiscard]] virtual auto hasChildren() const -> bool;
    [[nodiscard]] virtual auto canFetchMore() const -> bool;
    virtual void fetchMore() {}

    [[nodiscard]] auto parent() const -> TreeItem * { return m_parent; }

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
    [[nodiscard]] auto firstChild() const -> TreeItem *;
    [[nodiscard]] auto lastChild() const -> TreeItem *;
    [[nodiscard]] auto level() const -> int;

    using const_iterator = QVector<TreeItem *>::const_iterator;
    using value_type = TreeItem *;
    [[nodiscard]] auto childCount() const -> int { return m_children.size(); }
    [[nodiscard]] auto indexInParent() const -> int;
    [[nodiscard]] auto childAt(int index) const -> TreeItem *;
    auto indexOf(const TreeItem *item) const -> int;
    [[nodiscard]] const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] const_iterator end() const { return m_children.end(); }
    [[nodiscard]] auto index() const -> QModelIndex;
    [[nodiscard]] auto model() const -> QAbstractItemModel *;

    void forSelectedChildren(const std::function<bool(TreeItem *)> &pred) const;
    void forAllChildren(const std::function<void(TreeItem *)> &pred) const;
    [[nodiscard]] auto findAnyChild(const std::function<bool(TreeItem *)> &pred) const
        -> TreeItem *;
    // like findAnyChild() but processes children in exact reverse order
    // (bottom to top, most inner children first)
    [[nodiscard]] auto reverseFindAnyChild(const std::function<bool(TreeItem *)> &pred) const
        -> TreeItem *;

    // Levels are 1-based: Child at Level 1 is an immediate child.
    void forChildrenAtLevel(int level, const std::function<void(TreeItem *)> &pred) const;
    [[nodiscard]] auto findChildAtLevel(int level, const std::function<bool(TreeItem *)> &pred) const
        -> TreeItem *;

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

    [[nodiscard]] auto rootItem() const -> TreeItem *;
    void setRootItem(TreeItem *item);
    [[nodiscard]] auto itemForIndex(const QModelIndex &) const -> TreeItem *;
    auto indexForItem(const TreeItem *needle) const -> QModelIndex;

    [[nodiscard]] auto rowCount(const QModelIndex &idx = QModelIndex()) const -> int override;
    [[nodiscard]] auto columnCount(const QModelIndex &idx) const -> int override;

    auto setData(const QModelIndex &idx, const QVariant &data, int role) -> bool override;
    [[nodiscard]] auto data(const QModelIndex &idx, int role) const -> QVariant override;
    [[nodiscard]] auto index(int, int, const QModelIndex &idx = QModelIndex()) const
        -> QModelIndex override;
    [[nodiscard]] auto parent(const QModelIndex &idx) const -> QModelIndex override;
    [[nodiscard]] auto sibling(int row, int column, const QModelIndex &idx) const
        -> QModelIndex override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &idx) const override;
    [[nodiscard]] auto headerData(int section, Qt::Orientation orientation, int role) const
        -> QVariant override;
    [[nodiscard]] auto hasChildren(const QModelIndex &idx) const -> bool override;

    [[nodiscard]] auto canFetchMore(const QModelIndex &idx) const -> bool override;
    void fetchMore(const QModelIndex &idx) override;

    auto takeItem(TreeItem *item) -> TreeItem *; // item is not destroyed.
    void destroyItem(TreeItem *item);            // item is destroyed.

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
