#include "normaltreeview.hpp"
#include "fileitem.hpp"

#include <QHeaderView>

NormalTreeModel::NormalTreeModel(QObject *parent)
    : BaseTreeModel(parent)
{
    setTr();
}

void NormalTreeModel::setTr()
{
    m_headerList
        = QStringList{"", tr("名称"), tr("位置"), tr("修改时间"), tr("文件类型"), tr("大小")};
}

void NormalTreeModel::setDatas(const QFileInfoList &fileInfos, bool depth)
{
    clear();
    TreeItem *root = rootItem();
    for (const auto &fileInfo : qAsConst(fileInfos)) {
        root->appendChild(new FileItem(fileInfo, depth));
    }

    emit layoutChanged();
}

auto NormalTreeModel::rowCount(const QModelIndex &idx) const -> int
{
    return BaseTreeModel::rowCount(idx);
}

auto NormalTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    -> QVariant
{
    switch (role) {
    case Qt::TextAlignmentRole: return Qt::AlignVCenter;
    case Qt::CheckStateRole: {
        switch (section) {
        case 0: return true;
        default: break;
        }
    } break;
    case Qt::WhatsThisRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
    case Qt::DisplayRole: {
        if (section >= 0 && section < m_headerList.size()) {
            return m_headerList.at(section);
        }
    } break;
    default: break;
    }
    return {};
}

class NormalTreeView::NormalTreeViewPrivate
{
public:
    explicit NormalTreeViewPrivate(QWidget *parent)
        : owner(parent)
        , model(new NormalTreeModel(owner))
    {}

    QWidget *owner;

    NormalTreeModel *model;
};

NormalTreeView::NormalTreeView(QWidget *parent)
    : TreeView(parent)
    , d_ptr(new NormalTreeViewPrivate(this))
{
    setupUI();
}

NormalTreeView::~NormalTreeView() = default;

void NormalTreeView::setDatas(const QFileInfoList &fileInfos)
{
    d_ptr->model->setDatas(fileInfos, true);
}

void NormalTreeView::setupUI()
{
    setModel(d_ptr->model);
    setStyleSheet(
        "QTreeView{border:none; show-decoration-selected: 0;}"
        "QTreeView::item{height:50px; border: 1px solid transparent;show-decoration-selected: 0;}"
        "QTreeView::item:hover{background:#EBF5FF;}"
        "QTreeView::item:selected{background: #b8ceff; color:#222222;}"
        "QTreeView::branch {selection-color: #b8ceff;}");
}
