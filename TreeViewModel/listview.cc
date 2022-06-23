#include "listview.hpp"
#include "checkboxheaderview.hpp"
#include "fileinfo.hpp"
#include "fileitem.hpp"
#include "normaltreeview.hpp"

class ListView::ListViewPrivate
{
public:
    ListViewPrivate(QWidget *parent)
        : owner(parent)
        , model(new NormalTreeModel(owner))
        , header(new CheckBoxHeaderView(Qt::Horizontal, owner))
    {}

    QWidget *owner;

    NormalTreeModel *model;
    CheckBoxHeaderView *header;
};

ListView::ListView(QWidget *parent)
    : TreeView(parent)
    , d_ptr(new ListViewPrivate(this))
{
    setupUI();
    buildConnect();
}

ListView::~ListView() {}

void ListView::setDatas(const QFileInfoList &fileInfos)
{
    d_ptr->model->setDatas(fileInfos, false);
}

void ListView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    //qDebug() << selected << deselected;
    const QModelIndexList selectedList(selected.indexes());
    for (auto index : qAsConst(selectedList)) {
        if (!index.isValid()) {
            continue;
        }
        auto item = static_cast<FileItem *>(index.internalPointer());
        if (item) {
            auto data = item->fileInfo();
            data.setChecked(true);
        }
    }

    const QModelIndexList deselectedList(deselected.indexes());
    for (auto index : qAsConst(deselectedList)) {
        if (!index.isValid()) {
            continue;
        }
        auto item = static_cast<FileItem *>(index.internalPointer());
        if (item) {
            auto data = item->fileInfo();
            data.setChecked(false);
        }
    }

    updateCheckState();
}

void ListView::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.isValid()) {
        return;
    }
    auto item = static_cast<FileItem *>(topLeft.internalPointer());
    if (!item) {
        return;
    }
    //qDebug() << topLeft;
    selectionModel()->blockSignals(true);
    selectionModel()->select(topLeft,
                             item->fileInfo().checked()
                                 ? QItemSelectionModel::Select | QItemSelectionModel::Rows
                                 : QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    selectionModel()->blockSignals(false);

    updateCheckState();
}

void ListView::setupUI()
{
    setModel(d_ptr->model);
    setHeader(d_ptr->header);
}

void ListView::buildConnect()
{
    connect(selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &ListView::onSelectionChanged);
    connect(d_ptr->model, &NormalTreeModel::dataChanged, this, &ListView::onDataChanged);
    connect(d_ptr->header, &CheckBoxHeaderView::stateChanged, this, &ListView::onCheckAll);
}

void ListView::updateCheckState()
{
    update();
    int count = selectedIndexes().size();
    //qDebug() << count << d_ptr->model->rowCount();
    if (count == 0) {
        d_ptr->header->setState(Qt::Unchecked);
    } else if (count == d_ptr->model->rowCount()) {
        d_ptr->header->setState(Qt::Checked);
    } else {
        d_ptr->header->setState(Qt::PartiallyChecked);
    }
}
